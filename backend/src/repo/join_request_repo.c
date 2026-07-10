#include "join_request_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
join_request_repo_t *jr_repo_init(db_t *db){join_request_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int jr_repo_prepare(join_request_repo_t *r){
    if(!db_prepare(r->db,"jr_create","INSERT INTO join_requests(alliance_id,user_id,status) VALUES($1,$2,'pending') ON CONFLICT(alliance_id,user_id) DO NOTHING"))return 0;
    if(!db_prepare(r->db,"jr_find_al","SELECT jr.id,jr.alliance_id,jr.user_id,u.username,jr.status,EXTRACT(EPOCH FROM jr.created_at) FROM join_requests jr JOIN users u ON jr.user_id=u.id WHERE jr.alliance_id=$1 AND jr.status='pending'"))return 0;
    if(!db_prepare(r->db,"jr_find_uid","SELECT id,alliance_id,user_id,'',status,EXTRACT(EPOCH FROM created_at) FROM join_requests WHERE user_id=$1 AND status='pending'"))return 0;
    if(!db_prepare(r->db,"jr_update","UPDATE join_requests SET status=$3 WHERE alliance_id=$1 AND user_id=$2"))return 0;
    return 1;
}
int jr_repo_create(join_request_repo_t *r,entity_id_t aid,entity_id_t uid){
    char a[32],u[32];snprintf(a,sizeof(a),"%lld",(long long)aid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[2]={a,u};PGresult *res=PQexecPrepared(r->db->conn,"jr_create",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int jr_repo_find_by_alliance(join_request_repo_t *r,entity_id_t aid,join_request_t *out,int *cnt,int max){
    char a[32];snprintf(a,sizeof(a),"%lld",(long long)aid);
    const char *p[1]={a};PGresult *res=PQexecPrepared(r->db->conn,"jr_find_al",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        out[i].id=atoll(PQgetvalue(res,i,0));out[i].alliance_id=atoll(PQgetvalue(res,i,1));
        out[i].user_id=atoll(PQgetvalue(res,i,2));strncpy(out[i].username,PQgetvalue(res,i,3),sizeof(out[i].username)-1);
        strncpy(out[i].status,PQgetvalue(res,i,4),sizeof(out[i].status)-1);
        out[i].created_at=(time_t)atol(PQgetvalue(res,i,5));
    }PQclear(res);return 1;
}
int jr_repo_find_pending_by_user(join_request_repo_t *r,entity_id_t uid,join_request_t *out){
    char u[32];snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[1]={u};PGresult *res=PQexecPrepared(r->db->conn,"jr_find_uid",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));out->alliance_id=atoll(PQgetvalue(res,0,1));out->user_id=atoll(PQgetvalue(res,0,2));
    strncpy(out->status,PQgetvalue(res,0,4),sizeof(out->status)-1);out->created_at=(time_t)atol(PQgetvalue(res,0,5));
    PQclear(res);return 1;
}
int jr_repo_update_status(join_request_repo_t *r,entity_id_t aid,entity_id_t uid,const char *status){
    char a[32],u[32];snprintf(a,sizeof(a),"%lld",(long long)aid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[3]={a,u,status};PGresult *res=PQexecPrepared(r->db->conn,"jr_update",3,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
