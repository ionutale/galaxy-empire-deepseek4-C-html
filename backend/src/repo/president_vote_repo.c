#include "president_vote_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
president_vote_repo_t *pv_repo_init(db_t *db){president_vote_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int pv_repo_prepare(president_vote_repo_t *r){
    if(!db_prepare(r->db,"pv_create","INSERT INTO president_votes(alliance_id,initiator_id,expires_at) VALUES($1,$2,to_timestamp($3)) RETURNING id"))return 0;
    if(!db_prepare(r->db,"pv_active","SELECT id,alliance_id,initiator_id,EXTRACT(EPOCH FROM expires_at),status,yea_count,nay_count,EXTRACT(EPOCH FROM created_at) FROM president_votes WHERE alliance_id=$1 AND status='active' AND expires_at>CURRENT_TIMESTAMP ORDER BY created_at DESC LIMIT 1"))return 0;
    if(!db_prepare(r->db,"pv_status","UPDATE president_votes SET status=$2 WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"pv_vote","INSERT INTO president_vote_votes(vote_id,user_id,vote) VALUES($1,$2,$3)"))return 0;
    if(!db_prepare(r->db,"pv_has","SELECT 1 FROM president_vote_votes WHERE vote_id=$1 AND user_id=$2"))return 0;
    return 1;
}
int pv_repo_create(president_vote_repo_t *r,entity_id_t aid,entity_id_t iid,time_t exp,entity_id_t *out_id){
    char a[32],iidstr[32],ts[16];snprintf(a,sizeof(a),"%lld",(long long)aid);snprintf(iidstr,sizeof(iidstr),"%lld",(long long)iid);snprintf(ts,sizeof(ts),"%ld",(long)exp);
    const char *p[3]={a,iidstr,ts};PGresult *res=PQexecPrepared(r->db->conn,"pv_create",3,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);return 0;}
    *out_id=atoll(PQgetvalue(res,0,0));PQclear(res);return 1;
}
int pv_repo_find_active(president_vote_repo_t *r,entity_id_t aid,president_vote_t *out){
    char a[32];snprintf(a,sizeof(a),"%lld",(long long)aid);
    const char *p[1]={a};PGresult *res=PQexecPrepared(r->db->conn,"pv_active",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));out->alliance_id=atoll(PQgetvalue(res,0,1));out->initiator_id=atoll(PQgetvalue(res,0,2));
    out->expires_at=(time_t)atol(PQgetvalue(res,0,3));strncpy(out->status,PQgetvalue(res,0,4),sizeof(out->status)-1);
    out->yea_count=atoi(PQgetvalue(res,0,5));out->nay_count=atoi(PQgetvalue(res,0,6));
    out->created_at=(time_t)atol(PQgetvalue(res,0,7));PQclear(res);return 1;
}
int pv_repo_update_status(president_vote_repo_t *r,entity_id_t id,const char *status){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[2]={idstr,status};PGresult *res=PQexecPrepared(r->db->conn,"pv_status",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int pv_repo_add_vote(president_vote_repo_t *r,entity_id_t vid,entity_id_t uid,const char *vote){
    char v[32],u[32];snprintf(v,sizeof(v),"%lld",(long long)vid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[3]={v,u,vote};PGresult *res=PQexecPrepared(r->db->conn,"pv_vote",3,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int pv_repo_has_voted(president_vote_repo_t *r,entity_id_t vid,entity_id_t uid){
    char v[32],u[32];snprintf(v,sizeof(v),"%lld",(long long)vid);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[2]={v,u};PGresult *res=PQexecPrepared(r->db->conn,"pv_has",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_TUPLES_OK;PQclear(res);return ok;
}
