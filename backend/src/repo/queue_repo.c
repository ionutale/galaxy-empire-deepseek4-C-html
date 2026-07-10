#include "queue_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
queue_repo_t *queue_repo_init(db_t *db){queue_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int queue_repo_prepare(queue_repo_t *r){
    if(!db_prepare(r->db,"qi_con_in","INSERT INTO construction_queue_items(planet_id,building_name,from_level,to_level,sort_order) VALUES($1,$2,$3,$4,(SELECT COALESCE(MAX(sort_order),0)+1 FROM construction_queue_items WHERE planet_id=$1))"))return 0;
    if(!db_prepare(r->db,"qi_con_get","SELECT id,building_name,to_level FROM construction_queue_items WHERE planet_id=$1 ORDER BY sort_order"))return 0;
    if(!db_prepare(r->db,"qi_con_del","DELETE FROM construction_queue_items WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"qi_con_pop","DELETE FROM construction_queue_items WHERE id=(SELECT id FROM construction_queue_items WHERE planet_id=$1 ORDER BY sort_order LIMIT 1) RETURNING id,building_name,from_level,to_level"))return 0;
    if(!db_prepare(r->db,"qi_pr_in","INSERT INTO production_queue_items(planet_id,unit_type,quantity,sort_order) VALUES($1,$2,$3,(SELECT COALESCE(MAX(sort_order),0)+1 FROM production_queue_items WHERE planet_id=$1))"))return 0;
    if(!db_prepare(r->db,"qi_pr_get","SELECT id,unit_type,quantity FROM production_queue_items WHERE planet_id=$1 ORDER BY sort_order"))return 0;
    if(!db_prepare(r->db,"qi_pr_del","DELETE FROM production_queue_items WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"qi_pr_pop","DELETE FROM production_queue_items WHERE id=(SELECT id FROM production_queue_items WHERE planet_id=$1 ORDER BY sort_order LIMIT 1) RETURNING id,unit_type,quantity"))return 0;
    if(!db_prepare(r->db,"qi_re_in","INSERT INTO research_queue_items(user_id,tech_name,from_level,to_level,sort_order) VALUES($1,$2,0,1,(SELECT COALESCE(MAX(sort_order),0)+1 FROM research_queue_items WHERE user_id=$1))"))return 0;
    if(!db_prepare(r->db,"qi_re_get","SELECT id,tech_name FROM research_queue_items WHERE user_id=$1 ORDER BY sort_order"))return 0;
    if(!db_prepare(r->db,"qi_re_del","DELETE FROM research_queue_items WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"qi_re_pop","DELETE FROM research_queue_items WHERE id=(SELECT id FROM research_queue_items WHERE user_id=$1 ORDER BY sort_order LIMIT 1) RETURNING id,tech_name,from_level,to_level"))return 0;
    return 1;
}
int queue_repo_insert_construction(queue_repo_t *r,entity_id_t pid,const char *b,int fl,int tl){
    char p[32],fr[8],to[8];snprintf(p,sizeof(p),"%lld",(long long)pid);snprintf(fr,sizeof(fr),"%d",fl);snprintf(to,sizeof(to),"%d",tl);
    const char *params[4]={p,b,fr,to};PGresult *res=PQexecPrepared(r->db->conn,"qi_con_in",4,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int queue_repo_get_construction(queue_repo_t *r,entity_id_t pid,queue_item_t *out,int *cnt,int max){
    char p[32];snprintf(p,sizeof(p),"%lld",(long long)pid);
    const char *params[1]={p};PGresult *res=PQexecPrepared(r->db->conn,"qi_con_get",1,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){out[i].id=atoll(PQgetvalue(res,i,0));strncpy(out[i].building,PQgetvalue(res,i,1),sizeof(out[i].building)-1);out[i].target_level=atoi(PQgetvalue(res,i,2));}
    PQclear(res);return 1;
}
int queue_repo_delete_construction(queue_repo_t *r,entity_id_t id){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"qi_con_del",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int queue_repo_pop_first_construction(queue_repo_t *r,entity_id_t pid,queue_item_t *out){
    char p[32];snprintf(p,sizeof(p),"%lld",(long long)pid);
    const char *params[1]={p};PGresult *res=PQexecPrepared(r->db->conn,"qi_con_pop",1,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){memset(out,0,sizeof(*out));PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));strncpy(out->building,PQgetvalue(res,0,1),sizeof(out->building)-1);out->target_level=atoi(PQgetvalue(res,0,3));
    PQclear(res);return 1;
}
int queue_repo_insert_production(queue_repo_t *r,entity_id_t pid,const char *u,int qty){
    char p[32],q[8];snprintf(p,sizeof(p),"%lld",(long long)pid);snprintf(q,sizeof(q),"%d",qty);
    const char *params[3]={p,u,q};PGresult *res=PQexecPrepared(r->db->conn,"qi_pr_in",3,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int queue_repo_get_production(queue_repo_t *r,entity_id_t pid,prod_queue_item_t *out,int *cnt,int max){
    char p[32];snprintf(p,sizeof(p),"%lld",(long long)pid);
    const char *params[1]={p};PGresult *res=PQexecPrepared(r->db->conn,"qi_pr_get",1,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){out[i].id=atoll(PQgetvalue(res,i,0));strncpy(out[i].unit_type,PQgetvalue(res,i,1),sizeof(out[i].unit_type)-1);out[i].quantity=atoi(PQgetvalue(res,i,2));}
    PQclear(res);return 1;
}
int queue_repo_delete_production(queue_repo_t *r,entity_id_t id){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"qi_pr_del",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int queue_repo_pop_first_production(queue_repo_t *r,entity_id_t pid,prod_queue_item_t *out){
    char p[32];snprintf(p,sizeof(p),"%lld",(long long)pid);
    const char *params[1]={p};PGresult *res=PQexecPrepared(r->db->conn,"qi_pr_pop",1,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){memset(out,0,sizeof(*out));PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));strncpy(out->unit_type,PQgetvalue(res,0,1),sizeof(out->unit_type)-1);out->quantity=atoi(PQgetvalue(res,0,2));
    PQclear(res);return 1;
}
int queue_repo_insert_research(queue_repo_t *r,entity_id_t uid,const char *tech){
    char u[32];snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *params[2]={u,tech};PGresult *res=PQexecPrepared(r->db->conn,"qi_re_in",2,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int queue_repo_get_research(queue_repo_t *r,entity_id_t uid,research_queue_item_t *out,int *cnt,int max){
    char u[32];snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *params[1]={u};PGresult *res=PQexecPrepared(r->db->conn,"qi_re_get",1,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){out[i].id=atoll(PQgetvalue(res,i,0));strncpy(out[i].tech,PQgetvalue(res,i,1),sizeof(out[i].tech)-1);}
    PQclear(res);return 1;
}
int queue_repo_delete_research(queue_repo_t *r,entity_id_t id){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"qi_re_del",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int queue_repo_pop_first_research(queue_repo_t *r,entity_id_t uid,research_queue_item_t *out){
    char u[32];snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *params[1]={u};PGresult *res=PQexecPrepared(r->db->conn,"qi_re_pop",1,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){memset(out,0,sizeof(*out));PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));strncpy(out->tech,PQgetvalue(res,0,1),sizeof(out->tech)-1);
    PQclear(res);return 1;
}
