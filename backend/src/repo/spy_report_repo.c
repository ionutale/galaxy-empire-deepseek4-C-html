#include "spy_report_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
spy_report_repo_t *sr_repo_init(db_t *db){spy_report_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int sr_repo_prepare(spy_report_repo_t *r){
    if(!db_prepare(r->db,"sr_create","INSERT INTO spy_reports(attacker_id,target_id,coordinate_id,metal,crystal,gas) VALUES($1,$2,$3,$4,$5,$6)"))return 0;
    if(!db_prepare(r->db,"sr_find","SELECT id,attacker_id,target_id,coordinate_id,EXTRACT(EPOCH FROM timestamp),metal,crystal,gas,fleet_json,defenses_json,buildings_json,research_json FROM spy_reports WHERE attacker_id=$1 ORDER BY timestamp DESC LIMIT 50"))return 0;
    return 1;
}
int sr_repo_create(spy_report_repo_t *r,const spy_report_t *s){
    char aid[32],tid[32],cid[32],me[16],cry[16],ga[16];
    snprintf(aid,sizeof(aid),"%lld",(long long)s->attacker_id);
    snprintf(tid,sizeof(tid),"%lld",(long long)s->target_id); snprintf(cid,sizeof(cid),"%lld",(long long)s->coordinate_id);
    snprintf(me,sizeof(me),"%d",s->metal); snprintf(cry,sizeof(cry),"%d",s->crystal); snprintf(ga,sizeof(ga),"%d",s->gas);
    const char *p[6]={aid,tid,cid,me,cry,ga}; PGresult *res=PQexecPrepared(r->db->conn,"sr_create",6,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
int sr_repo_find_by_attacker(spy_report_repo_t *r,entity_id_t aid,spy_report_t *out,int *cnt,int max){
    char id[32]; snprintf(id,sizeof(id),"%lld",(long long)aid);
    const char *p[1]={id}; PGresult *res=PQexecPrepared(r->db->conn,"sr_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res); if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        out[i].id=atoll(PQgetvalue(res,i,0));out[i].attacker_id=atoll(PQgetvalue(res,i,1));
        out[i].target_id=atoll(PQgetvalue(res,i,2));out[i].coordinate_id=atoll(PQgetvalue(res,i,3));
        out[i].timestamp=(time_t)atol(PQgetvalue(res,i,4));out[i].metal=atoi(PQgetvalue(res,i,5));
        out[i].crystal=atoi(PQgetvalue(res,i,6));out[i].gas=atoi(PQgetvalue(res,i,7));
    } PQclear(res); return 1;
}
