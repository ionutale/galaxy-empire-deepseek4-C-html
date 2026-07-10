#include "fortress_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
fortress_repo_t *fortress_repo_init(db_t *db){fortress_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int fortress_repo_prepare(fortress_repo_t *r){
    if(!db_prepare(r->db,"fo_count","SELECT COALESCE(SUM(count),0) FROM fortress_attacks WHERE user_id=$1 AND attack_date=$2"))return 0;
    if(!db_prepare(r->db,"fo_inc","INSERT INTO fortress_attacks(user_id,attack_date,count) VALUES($1,$2,1) ON CONFLICT(user_id,attack_date) DO UPDATE SET count=fortress_attacks.count+1"))return 0;
    if(!db_prepare(r->db,"fo_get","SELECT alliance_id FROM fortress_control LIMIT 1"))return 0;
    if(!db_prepare(r->db,"fo_del","DELETE FROM fortress_control"))return 0;
    if(!db_prepare(r->db,"fo_ins","INSERT INTO fortress_control(alliance_id) VALUES($1)"))return 0;
    if(!db_prepare(r->db,"fo_init","INSERT INTO fortress_control(alliance_id) VALUES(NULL) ON CONFLICT DO NOTHING"))return 0;
    return 1;
}
int fortress_repo_get_attack_count(fortress_repo_t *r,entity_id_t uid,const char *date){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[2]={id,date};PGresult *res=PQexecPrepared(r->db->conn,"fo_count",2,p,NULL,NULL,0);
    int n=0; if(PQresultStatus(res)==PGRES_TUPLES_OK) n=atoi(PQgetvalue(res,0,0)); PQclear(res); return n;
}
int fortress_repo_increment_attacks(fortress_repo_t *r,entity_id_t uid,const char *date){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[2]={id,date};PGresult *res=PQexecPrepared(r->db->conn,"fo_inc",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
int fortress_repo_get_control(fortress_repo_t *r,entity_id_t *aid){
    PGresult *res=PQexecPrepared(r->db->conn,"fo_get",0,NULL,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){*aid=0;PQclear(res);return 1;}
    if(PQgetisnull(res,0,0)){*aid=0;PQclear(res);return 1;}
    *aid=atoll(PQgetvalue(res,0,0)); PQclear(res); return 1;
}
int fortress_repo_set_control(fortress_repo_t *r,entity_id_t aid){
    PGresult *res=PQexecPrepared(r->db->conn,"fo_del",0,NULL,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); if(!ok) return 0;
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)aid);
    const char *p[1]={id};res=PQexecPrepared(r->db->conn,"fo_ins",1,p,NULL,NULL,0);
    ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
int fortress_repo_clear_control(fortress_repo_t *r){
    PGresult *res=PQexecPrepared(r->db->conn,"fo_del",0,NULL,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); if(!ok) return 0;
    const char *p[1]={NULL};res=PQexecPrepared(r->db->conn,"fo_ins",1,p,NULL,NULL,0);
    ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
int fortress_repo_init_control(fortress_repo_t *r){
    PGresult *res=PQexecPrepared(r->db->conn,"fo_init",0,NULL,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
