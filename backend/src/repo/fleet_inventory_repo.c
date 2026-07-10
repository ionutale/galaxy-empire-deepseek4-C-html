#include "fleet_inventory_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
fleet_inventory_repo_t *fleet_inv_repo_init(db_t *db) { fleet_inventory_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int fleet_inv_repo_prepare(fleet_inventory_repo_t *r){
    if(!db_prepare(r->db,"fi_find","SELECT light_fighter,heavy_fighter,cruiser,battleship,dreadnought,destroyer,bomber,interceptor,iron_behemoth,devourer_interceptor,titanenergetic_frigate,annihilator,small_cargo,large_cargo,recycler,colony_ship,probe,solar_satellite FROM fleet_inventories WHERE planet_id=$1"))return 0;
    if(!db_prepare(r->db,"fi_upsert","INSERT INTO fleet_inventories(planet_id,light_fighter,heavy_fighter,cruiser,battleship,dreadnought,destroyer,bomber,interceptor,iron_behemoth,devourer_interceptor,titanenergetic_frigate,annihilator,small_cargo,large_cargo,recycler,colony_ship,probe,solar_satellite) VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17,$18,$19) ON CONFLICT(planet_id) DO UPDATE SET light_fighter=EXCLUDED.light_fighter,heavy_fighter=EXCLUDED.heavy_fighter,cruiser=EXCLUDED.cruiser,battleship=EXCLUDED.battleship,dreadnought=EXCLUDED.dreadnought,destroyer=EXCLUDED.destroyer,bomber=EXCLUDED.bomber,interceptor=EXCLUDED.interceptor,iron_behemoth=EXCLUDED.iron_behemoth,devourer_interceptor=EXCLUDED.devourer_interceptor,titanenergetic_frigate=EXCLUDED.titanenergetic_frigate,annihilator=EXCLUDED.annihilator,small_cargo=EXCLUDED.small_cargo,large_cargo=EXCLUDED.large_cargo,recycler=EXCLUDED.recycler,colony_ship=EXCLUDED.colony_ship,probe=EXCLUDED.probe,solar_satellite=EXCLUDED.solar_satellite"))return 0;
    return 1;
}
int fleet_inv_repo_find(fleet_inventory_repo_t *r,entity_id_t pid,fleet_ships_t *out){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)pid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"fi_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);memset(out,0,sizeof(*out));return 0;}
    out->light_fighter=atoi(PQgetvalue(res,0,0));out->heavy_fighter=atoi(PQgetvalue(res,0,1));
    out->cruiser=atoi(PQgetvalue(res,0,2));out->battleship=atoi(PQgetvalue(res,0,3));
    out->dreadnought=atoi(PQgetvalue(res,0,4));out->destroyer=atoi(PQgetvalue(res,0,5));
    out->bomber=atoi(PQgetvalue(res,0,6));out->interceptor=atoi(PQgetvalue(res,0,7));
    out->iron_behemoth=atoi(PQgetvalue(res,0,8));out->devourer_interceptor=atoi(PQgetvalue(res,0,9));
    out->titanenergetic_frigate=atoi(PQgetvalue(res,0,10));out->annihilator=atoi(PQgetvalue(res,0,11));
    out->small_cargo=atoi(PQgetvalue(res,0,12));out->large_cargo=atoi(PQgetvalue(res,0,13));
    out->recycler=atoi(PQgetvalue(res,0,14));out->colony_ship=atoi(PQgetvalue(res,0,15));
    out->probe=atoi(PQgetvalue(res,0,16));out->solar_satellite=atoi(PQgetvalue(res,0,17));
    PQclear(res);return 1;
}
int fleet_inv_repo_upsert(fleet_inventory_repo_t *r,entity_id_t pid,const fleet_ships_t *s){
    char id[32],lf[16],hf[16],cr[16],bs[16],dr[16],de[16],bo[16],in[16],ib[16],di[16],tf[16],an[16],sc[16],lc[16],re[16],co[16],pr[16],ss[16];
    snprintf(id,sizeof(id),"%lld",(long long)pid);
    #define SI(f) snprintf(lf,sizeof(lf),"%d",s->f)
    snprintf(lf,sizeof(lf),"%d",s->light_fighter);snprintf(hf,sizeof(hf),"%d",s->heavy_fighter);
    snprintf(cr,sizeof(cr),"%d",s->cruiser);snprintf(bs,sizeof(bs),"%d",s->battleship);
    snprintf(dr,sizeof(dr),"%d",s->dreadnought);snprintf(de,sizeof(de),"%d",s->destroyer);
    snprintf(bo,sizeof(bo),"%d",s->bomber);snprintf(in,sizeof(in),"%d",s->interceptor);
    snprintf(ib,sizeof(ib),"%d",s->iron_behemoth);snprintf(di,sizeof(di),"%d",s->devourer_interceptor);
    snprintf(tf,sizeof(tf),"%d",s->titanenergetic_frigate);snprintf(an,sizeof(an),"%d",s->annihilator);
    snprintf(sc,sizeof(sc),"%d",s->small_cargo);snprintf(lc,sizeof(lc),"%d",s->large_cargo);
    snprintf(re,sizeof(re),"%d",s->recycler);snprintf(co,sizeof(co),"%d",s->colony_ship);
    snprintf(pr,sizeof(pr),"%d",s->probe);snprintf(ss,sizeof(ss),"%d",s->solar_satellite);
    const char *params[19]={id,lf,hf,cr,bs,dr,de,bo,in,ib,di,tf,an,sc,lc,re,co,pr,ss};
    PGresult *res=PQexecPrepared(r->db->conn,"fi_upsert",19,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
