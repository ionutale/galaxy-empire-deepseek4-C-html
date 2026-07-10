#include "commander_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
commander_repo_t *commander_repo_init(db_t *db){commander_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int commander_repo_prepare(commander_repo_t *r){
    if(!db_prepare(r->db,"com_find","SELECT id,user_id,commander_type,tier,EXTRACT(EPOCH FROM expires_at) FROM commanders WHERE user_id=$1 AND expires_at>CURRENT_TIMESTAMP"))return 0;
    if(!db_prepare(r->db,"com_find_type","SELECT id,user_id,commander_type,tier,EXTRACT(EPOCH FROM expires_at) FROM commanders WHERE user_id=$1 AND commander_type=$2 AND expires_at>CURRENT_TIMESTAMP"))return 0;
    if(!db_prepare(r->db,"com_hire","INSERT INTO commanders(user_id,commander_type,tier,expires_at) VALUES($1,$2,$3,CURRENT_TIMESTAMP+make_interval(days=>$4::int)) ON CONFLICT(user_id,commander_type) DO UPDATE SET tier=EXCLUDED.tier,expires_at=EXCLUDED.expires_at"))return 0;
    return 1;
}
int commander_repo_find_active(commander_repo_t *r,entity_id_t uid,commander_t *out,int *cnt,int max){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"com_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);*cnt=0;return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        out[i].id=atoll(PQgetvalue(res,i,0));out[i].user_id=atoll(PQgetvalue(res,i,1));
        strncpy(out[i].commander_type,PQgetvalue(res,i,2),sizeof(out[i].commander_type)-1);
        out[i].tier=atoi(PQgetvalue(res,i,3));out[i].expires_at=(time_t)atol(PQgetvalue(res,i,4));
    }PQclear(res);return 1;
}
int commander_repo_find_active_by_type(commander_repo_t *r,entity_id_t uid,const char *type,commander_t *out){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[2]={id,type};PGresult *res=PQexecPrepared(r->db->conn,"com_find_type",2,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));out->user_id=atoll(PQgetvalue(res,0,1));
    strncpy(out->commander_type,PQgetvalue(res,0,2),sizeof(out->commander_type)-1);
    out->tier=atoi(PQgetvalue(res,0,3));out->expires_at=(time_t)atol(PQgetvalue(res,0,4));
    PQclear(res);return 1;
}
int commander_repo_hire(commander_repo_t *r,entity_id_t uid,const char *type,int tier,time_t expires_at){
    char id[32],ti[16],days[16];
    snprintf(id,sizeof(id),"%lld",(long long)uid);snprintf(ti,sizeof(ti),"%d",tier);
    snprintf(days,sizeof(days),"%ld",(expires_at-time(NULL))/86400);
    const char *p[4]={id,type,ti,days};PGresult *res=PQexecPrepared(r->db->conn,"com_hire",4,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
