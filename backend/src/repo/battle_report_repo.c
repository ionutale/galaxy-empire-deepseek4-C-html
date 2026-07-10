#include "battle_report_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
battle_report_repo_t *br_repo_init(db_t *db){battle_report_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int br_repo_prepare(battle_report_repo_t *r){
    if(!db_prepare(r->db,"br_create","INSERT INTO battle_reports(attacker_id,defender_id,coordinate_id,outcome,rounds_json,debris_metal,debris_crystal,moon_chance,moon_created,loot_metal,loot_crystal,loot_gas) VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12)"))return 0;
    if(!db_prepare(r->db,"br_find_player","SELECT id,attacker_id,defender_id,coordinate_id,EXTRACT(EPOCH FROM timestamp),outcome,debris_metal,debris_crystal,moon_chance,moon_created,loot_metal,loot_crystal,loot_gas FROM battle_reports WHERE attacker_id=$1 OR defender_id=$1 ORDER BY timestamp DESC LIMIT 50"))return 0;
    if(!db_prepare(r->db,"br_find_id","SELECT id,attacker_id,defender_id,coordinate_id,EXTRACT(EPOCH FROM timestamp),outcome,rounds_json,debris_metal,debris_crystal,moon_chance,moon_created,loot_metal,loot_crystal,loot_gas FROM battle_reports WHERE id=$1"))return 0;
    return 1;
}
int br_repo_create(battle_report_repo_t *r,const battle_report_t *report){
    char aid[32],did[32],cid[32],dm[32],dc[32],mc[32],lm[32],lc[32],lg[32];
    snprintf(aid,sizeof(aid),"%lld",(long long)report->attacker_id);
    snprintf(did,sizeof(did),"%lld",(long long)report->defender_id);snprintf(cid,sizeof(cid),"%lld",(long long)report->coordinate_id);
    snprintf(lm,sizeof(lm),"%.0f",report->loot_metal);snprintf(lc,sizeof(lc),"%.0f",report->loot_crystal);snprintf(lg,sizeof(lg),"%.0f",report->loot_gas);
    snprintf(dm,sizeof(dm),"%lld",(long long)report->debris_metal);snprintf(dc,sizeof(dc),"%lld",(long long)report->debris_crystal);
    snprintf(mc,sizeof(mc),"%f",report->moon_chance);
    const char *mn=report->moon_created?"true":"false";
    const char *params[12]={aid,did,cid,report->outcome,report->rounds_json,dm,dc,mc,mn,lm,lc,lg};
    PGresult *res=PQexecPrepared(r->db->conn,"br_create",12,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int br_repo_find_by_player(battle_report_repo_t *r,entity_id_t pid,battle_report_t *out,int *cnt,int max){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)pid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"br_find_player",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);*cnt=0;return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        out[i].id=atoll(PQgetvalue(res,i,0));out[i].attacker_id=atoll(PQgetvalue(res,i,1));
        out[i].defender_id=atoll(PQgetvalue(res,i,2));out[i].coordinate_id=atoll(PQgetvalue(res,i,3));
        out[i].timestamp=(time_t)atol(PQgetvalue(res,i,4));strncpy(out[i].outcome,PQgetvalue(res,i,5),sizeof(out[i].outcome)-1);
        out[i].debris_metal=atoll(PQgetvalue(res,i,6));out[i].debris_crystal=atoll(PQgetvalue(res,i,7));
        out[i].moon_chance=atof(PQgetvalue(res,i,8));out[i].moon_created=strcmp(PQgetvalue(res,i,9),"t")==0;
        out[i].loot_metal=atof(PQgetvalue(res,i,10));out[i].loot_crystal=atof(PQgetvalue(res,i,11));out[i].loot_gas=atof(PQgetvalue(res,i,12));
    }PQclear(res);return 1;
}
int br_repo_find_by_id(battle_report_repo_t *r,entity_id_t id,battle_report_t *out){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"br_find_id",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));out->attacker_id=atoll(PQgetvalue(res,0,1));out->defender_id=atoll(PQgetvalue(res,0,2));
    out->coordinate_id=atoll(PQgetvalue(res,0,3));out->timestamp=(time_t)atol(PQgetvalue(res,0,4));
    strncpy(out->outcome,PQgetvalue(res,0,5),sizeof(out->outcome)-1);
    strncpy(out->rounds_json,PQgetvalue(res,0,6),sizeof(out->rounds_json)-1);
    out->debris_metal=atoll(PQgetvalue(res,0,7));out->debris_crystal=atoll(PQgetvalue(res,0,8));
    out->moon_chance=atof(PQgetvalue(res,0,9));out->moon_created=strcmp(PQgetvalue(res,0,10),"t")==0;
    out->loot_metal=atof(PQgetvalue(res,0,11));out->loot_crystal=atof(PQgetvalue(res,0,12));out->loot_gas=atof(PQgetvalue(res,0,13));
    PQclear(res);return 1;
}
