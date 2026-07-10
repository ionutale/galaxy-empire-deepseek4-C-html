#include "alliance_member_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
alliance_member_repo_t *am_repo_init(db_t *db){alliance_member_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int am_repo_prepare(alliance_member_repo_t *r){
    if(!db_prepare(r->db,"am_add","INSERT INTO alliance_members(alliance_id,user_id,role) VALUES($1,$2,$3)"))return 0;
    if(!db_prepare(r->db,"am_remove","DELETE FROM alliance_members WHERE alliance_id=$1 AND user_id=$2"))return 0;
    if(!db_prepare(r->db,"am_find_al","SELECT m.id,m.alliance_id,m.user_id,u.username,m.role FROM alliance_members m JOIN users u ON m.user_id=u.id WHERE m.alliance_id=$1"))return 0;
    if(!db_prepare(r->db,"am_find_uid","SELECT m.id,m.alliance_id,m.user_id,u.username,m.role FROM alliance_members m JOIN users u ON m.user_id=u.id WHERE m.user_id=$1"))return 0;
    if(!db_prepare(r->db,"am_role","UPDATE alliance_members SET role=$3 WHERE alliance_id=$1 AND user_id=$2"))return 0;
    if(!db_prepare(r->db,"am_count","SELECT COUNT(*) FROM alliance_members WHERE alliance_id=$1"))return 0;
    return 1;
}
int am_repo_add(alliance_member_repo_t *r,entity_id_t aid,entity_id_t uid,const char *role){
    char a[32],u[32];snprintf(a,sizeof(a),"%lld",(long long)aid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[3]={a,u,role};PGresult *res=PQexecPrepared(r->db->conn,"am_add",3,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int am_repo_remove(alliance_member_repo_t *r,entity_id_t aid,entity_id_t uid){
    char a[32],u[32];snprintf(a,sizeof(a),"%lld",(long long)aid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[2]={a,u};PGresult *res=PQexecPrepared(r->db->conn,"am_remove",2,p,NULL,NULL,0);int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int am_repo_find_by_alliance(alliance_member_repo_t *r,entity_id_t aid,alliance_member_t *out,int *cnt,int max){
    char a[32];snprintf(a,sizeof(a),"%lld",(long long)aid);
    const char *p[1]={a};PGresult *res=PQexecPrepared(r->db->conn,"am_find_al",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);*cnt=0;return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        out[i].id=atoll(PQgetvalue(res,i,0));out[i].alliance_id=atoll(PQgetvalue(res,i,1));
        out[i].user_id=atoll(PQgetvalue(res,i,2));strncpy(out[i].username,PQgetvalue(res,i,3),sizeof(out[i].username)-1);
        strncpy(out[i].role,PQgetvalue(res,i,4),sizeof(out[i].role)-1);
    }PQclear(res);return 1;
}
int am_repo_find_by_user(alliance_member_repo_t *r,entity_id_t uid,alliance_member_t *out){
    char u[32];snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[1]={u};PGresult *res=PQexecPrepared(r->db->conn,"am_find_uid",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));out->alliance_id=atoll(PQgetvalue(res,0,1));
    out->user_id=atoll(PQgetvalue(res,0,2));strncpy(out->username,PQgetvalue(res,0,3),sizeof(out->username)-1);
    strncpy(out->role,PQgetvalue(res,0,4),sizeof(out->role)-1);PQclear(res);return 1;
}
int am_repo_update_role(alliance_member_repo_t *r,entity_id_t aid,entity_id_t uid,const char *role){
    char a[32],u[32];snprintf(a,sizeof(a),"%lld",(long long)aid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[3]={a,u,role};PGresult *res=PQexecPrepared(r->db->conn,"am_role",3,p,NULL,NULL,0);int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int am_repo_count(alliance_member_repo_t *r,entity_id_t aid){
    char a[32];snprintf(a,sizeof(a),"%lld",(long long)aid);
    const char *p[1]={a};PGresult *res=PQexecPrepared(r->db->conn,"am_count",1,p,NULL,NULL,0);
    int n=PQresultStatus(res)==PGRES_TUPLES_OK?atoi(PQgetvalue(res,0,0)):0;PQclear(res);return n;
}
