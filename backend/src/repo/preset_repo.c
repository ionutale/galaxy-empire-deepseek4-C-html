#include "preset_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
preset_repo_t *preset_repo_init(db_t *db){preset_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int preset_repo_prepare(preset_repo_t *r){
    if(!db_prepare(r->db,"ps_find","SELECT id,name,light_fighter,heavy_fighter,cruiser,battleship,dreadnought,destroyer,bomber,interceptor,iron_behemoth,devourer_interceptor,titanenergetic_frigate,annihilator,small_cargo,large_cargo,recycler,colony_ship,probe,solar_satellite FROM fleet_presets WHERE user_id=$1 ORDER BY created_at"))return 0;
    if(!db_prepare(r->db,"ps_create","INSERT INTO fleet_presets(user_id,name,light_fighter,heavy_fighter,cruiser,battleship,dreadnought,destroyer,bomber,interceptor,iron_behemoth,devourer_interceptor,titanenergetic_frigate,annihilator,small_cargo,large_cargo,recycler,colony_ship,probe,solar_satellite) VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20) RETURNING id"))return 0;
    if(!db_prepare(r->db,"ps_del","DELETE FROM fleet_presets WHERE id=$1 AND user_id=$2"))return 0;
    return 1;
}
int preset_repo_find_by_user(preset_repo_t *r,entity_id_t uid,fleet_ships_t *ships,char (*names)[64],int *cnt,int max){
    char u[32];snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[1]={u};PGresult *res=PQexecPrepared(r->db->conn,"ps_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res);if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++){
        strncpy(names[i],PQgetvalue(res,i,1),63);
        ships[i].light_fighter=atoi(PQgetvalue(res,i,2));ships[i].heavy_fighter=atoi(PQgetvalue(res,i,3));
        ships[i].cruiser=atoi(PQgetvalue(res,i,4));ships[i].battleship=atoi(PQgetvalue(res,i,5));
        ships[i].dreadnought=atoi(PQgetvalue(res,i,6));ships[i].destroyer=atoi(PQgetvalue(res,i,7));
        ships[i].bomber=atoi(PQgetvalue(res,i,8));ships[i].interceptor=atoi(PQgetvalue(res,i,9));
        ships[i].iron_behemoth=atoi(PQgetvalue(res,i,10));ships[i].devourer_interceptor=atoi(PQgetvalue(res,i,11));
        ships[i].titanenergetic_frigate=atoi(PQgetvalue(res,i,12));ships[i].annihilator=atoi(PQgetvalue(res,i,13));
        ships[i].small_cargo=atoi(PQgetvalue(res,i,14));ships[i].large_cargo=atoi(PQgetvalue(res,i,15));
        ships[i].recycler=atoi(PQgetvalue(res,i,16));ships[i].colony_ship=atoi(PQgetvalue(res,i,17));
        ships[i].probe=atoi(PQgetvalue(res,i,18));ships[i].solar_satellite=atoi(PQgetvalue(res,i,19));
    }PQclear(res);return 1;
}
int preset_repo_create(preset_repo_t *r,entity_id_t uid,const char *name,const fleet_ships_t *s,entity_id_t *out_id){
    char u[32],lf[16],hf[16],cr[16],bs[16],dr[16],de[16],bo[16],in[16],ib[16],di[16],tf[16],an[16],sc[16],lc[16],re[16],co[16],pr[16],ss[16];
    snprintf(u,sizeof(u),"%lld",(long long)uid);
    snprintf(lf,sizeof(lf),"%d",s->light_fighter);snprintf(hf,sizeof(hf),"%d",s->heavy_fighter);
    snprintf(cr,sizeof(cr),"%d",s->cruiser);snprintf(bs,sizeof(bs),"%d",s->battleship);
    snprintf(dr,sizeof(dr),"%d",s->dreadnought);snprintf(de,sizeof(de),"%d",s->destroyer);
    snprintf(bo,sizeof(bo),"%d",s->bomber);snprintf(in,sizeof(in),"%d",s->interceptor);
    snprintf(ib,sizeof(ib),"%d",s->iron_behemoth);snprintf(di,sizeof(di),"%d",s->devourer_interceptor);
    snprintf(tf,sizeof(tf),"%d",s->titanenergetic_frigate);snprintf(an,sizeof(an),"%d",s->annihilator);
    snprintf(sc,sizeof(sc),"%d",s->small_cargo);snprintf(lc,sizeof(lc),"%d",s->large_cargo);
    snprintf(re,sizeof(re),"%d",s->recycler);snprintf(co,sizeof(co),"%d",s->colony_ship);
    snprintf(pr,sizeof(pr),"%d",s->probe);snprintf(ss,sizeof(ss),"%d",s->solar_satellite);
    const char *params[20]={u,name,lf,hf,cr,bs,dr,de,bo,in,ib,di,tf,an,sc,lc,re,co,pr,ss};
    PGresult *res=PQexecPrepared(r->db->conn,"ps_create",20,params,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);return 0;}
    *out_id=atoll(PQgetvalue(res,0,0));PQclear(res);return 1;
}
int preset_repo_delete(preset_repo_t *r,entity_id_t id,entity_id_t uid){
    char idstr[32],u[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);snprintf(u,sizeof(u),"%lld",(long long)uid);
    const char *p[2]={idstr,u};PGresult *res=PQexecPrepared(r->db->conn,"ps_del",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
