#include "galactonite_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
galactonite_repo_t *ga_repo_init(db_t *db){galactonite_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int ga_repo_prepare(galactonite_repo_t *r){
    if(!db_prepare(r->db,"ga_find","SELECT id,user_id,type_name,stars,level,equipped FROM galactonites WHERE user_id=$1"))return 0;
    if(!db_prepare(r->db,"ga_find_id","SELECT id,user_id,type_name,stars,level,equipped FROM galactonites WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"ga_create","INSERT INTO galactonites(user_id,type_name,stars,level) VALUES($1,$2,$3,$4) RETURNING id"))return 0;
    if(!db_prepare(r->db,"ga_lvl","UPDATE galactonites SET level=$2 WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"ga_eq","UPDATE galactonites SET equipped=$2 WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"ga_ce_get","SELECT amount FROM crystal_energy WHERE user_id=$1"))return 0;
    if(!db_prepare(r->db,"ga_ce_add","INSERT INTO crystal_energy(user_id,amount) VALUES($1,$2) ON CONFLICT(user_id) DO UPDATE SET amount=crystal_energy.amount+$2"))return 0;
    if(!db_prepare(r->db,"ga_ce_sub","UPDATE crystal_energy SET amount=amount-$2 WHERE user_id=$1 AND amount>=$2"))return 0;
    return 1;
}
int ga_repo_find_by_owner(galactonite_repo_t *r,entity_id_t uid,galactonite_t *out,int *cnt,int max){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"ga_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        out[i].id=atoll(PQgetvalue(res,i,0));out[i].user_id=atoll(PQgetvalue(res,i,1));
        strncpy(out[i].galactonite_type,PQgetvalue(res,i,2),sizeof(out[i].galactonite_type)-1);
        out[i].stars=atoi(PQgetvalue(res,i,3));out[i].level=atoi(PQgetvalue(res,i,4));
        out[i].equipped=strcmp(PQgetvalue(res,i,5),"t")==0;
    }PQclear(res);return 1;
}
int ga_repo_find_by_id(galactonite_repo_t *r,entity_id_t id,galactonite_t *out){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"ga_find_id",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));out->user_id=atoll(PQgetvalue(res,0,1));
    strncpy(out->galactonite_type,PQgetvalue(res,0,2),sizeof(out->galactonite_type)-1);
    out->stars=atoi(PQgetvalue(res,0,3));out->level=atoi(PQgetvalue(res,0,4));
    out->equipped=strcmp(PQgetvalue(res,0,5),"t")==0;PQclear(res);return 1;
}
int ga_repo_create(galactonite_repo_t *r,entity_id_t uid,const char *type,int stars,int level,entity_id_t *out_id){
    char u[32],st[8],lv[8];snprintf(u,sizeof(u),"%lld",(long long)uid);snprintf(st,sizeof(st),"%d",stars);snprintf(lv,sizeof(lv),"%d",level);
    const char *p[4]={u,type,st,lv};PGresult *res=PQexecPrepared(r->db->conn,"ga_create",4,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);return 0;}
    *out_id=atoll(PQgetvalue(res,0,0));PQclear(res);return 1;
}
int ga_repo_update_level(galactonite_repo_t *r,entity_id_t id,int level){
    char idstr[32],lv[8];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);snprintf(lv,sizeof(lv),"%d",level);
    const char *p[2]={idstr,lv};PGresult *res=PQexecPrepared(r->db->conn,"ga_lvl",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int ga_repo_update_equipped(galactonite_repo_t *r,entity_id_t id,int equipped){
    char idstr[32],eq[4];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);snprintf(eq,sizeof(eq),"%d",equipped);
    const char *p[2]={idstr,eq};PGresult *res=PQexecPrepared(r->db->conn,"ga_eq",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int ga_repo_find_crystal_energy(galactonite_repo_t *r,entity_id_t uid,int64_t *amount){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"ga_ce_get",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){*amount=0;PQclear(res);return 1;}
    *amount=atoll(PQgetvalue(res,0,0));PQclear(res);return 1;
}
int ga_repo_add_crystal_energy(galactonite_repo_t *r,entity_id_t uid,int64_t amount){
    char u[32],a[32];snprintf(u,sizeof(u),"%lld",(long long)uid);snprintf(a,sizeof(a),"%lld",(long long)amount);
    const char *p[2]={u,a};PGresult *res=PQexecPrepared(r->db->conn,"ga_ce_add",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int ga_repo_deduct_crystal_energy(galactonite_repo_t *r,entity_id_t uid,int64_t amount){
    char u[32],a[32];snprintf(u,sizeof(u),"%lld",(long long)uid);snprintf(a,sizeof(a),"%lld",(long long)amount);
    const char *p[2]={u,a};PGresult *res=PQexecPrepared(r->db->conn,"ga_ce_sub",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK&&PQcmdTuples(res)!=NULL&&atoi(PQcmdTuples(res))>0;PQclear(res);return ok;
}
