#include "defense_inventory_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
defense_inventory_repo_t *def_inv_repo_init(db_t *db){defense_inventory_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int def_inv_repo_prepare(defense_inventory_repo_t *r){
    if(!db_prepare(r->db,"di_find","SELECT rocket_launcher,light_laser,heavy_laser,ion_cannon,mk2_cannon,plasma_cannon,proton_cannon,small_shield_dome,large_shield_dome FROM defense_inventories WHERE planet_id=$1"))return 0;
    if(!db_prepare(r->db,"di_upsert","INSERT INTO defense_inventories(planet_id,rocket_launcher,light_laser,heavy_laser,ion_cannon,mk2_cannon,plasma_cannon,proton_cannon,small_shield_dome,large_shield_dome) VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9,$10) ON CONFLICT(planet_id) DO UPDATE SET rocket_launcher=EXCLUDED.rocket_launcher,light_laser=EXCLUDED.light_laser,heavy_laser=EXCLUDED.heavy_laser,ion_cannon=EXCLUDED.ion_cannon,mk2_cannon=EXCLUDED.mk2_cannon,plasma_cannon=EXCLUDED.plasma_cannon,proton_cannon=EXCLUDED.proton_cannon,small_shield_dome=EXCLUDED.small_shield_dome,large_shield_dome=EXCLUDED.large_shield_dome"))return 0;
    return 1;
}
int def_inv_repo_find(defense_inventory_repo_t *r,entity_id_t pid,defense_ships_t *out){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)pid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"di_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);memset(out,0,sizeof(*out));return 0;}
    out->rocket_launcher=atoi(PQgetvalue(res,0,0));out->light_laser=atoi(PQgetvalue(res,0,1));
    out->heavy_laser=atoi(PQgetvalue(res,0,2));out->ion_cannon=atoi(PQgetvalue(res,0,3));
    out->mk2_cannon=atoi(PQgetvalue(res,0,4));out->plasma_cannon=atoi(PQgetvalue(res,0,5));
    out->proton_cannon=atoi(PQgetvalue(res,0,6));out->small_shield_dome=atoi(PQgetvalue(res,0,7));
    out->large_shield_dome=atoi(PQgetvalue(res,0,8));
    PQclear(res);return 1;
}
int def_inv_repo_upsert(defense_inventory_repo_t *r,entity_id_t pid,const defense_ships_t *d){
    char id[32],rl[16],ll[16],hl[16],ic[16],mk[16],pc[16],prc[16],ssd[16],lsd[16];
    snprintf(id,sizeof(id),"%lld",(long long)pid);
    snprintf(rl,sizeof(rl),"%d",d->rocket_launcher);snprintf(ll,sizeof(ll),"%d",d->light_laser);
    snprintf(hl,sizeof(hl),"%d",d->heavy_laser);snprintf(ic,sizeof(ic),"%d",d->ion_cannon);
    snprintf(mk,sizeof(mk),"%d",d->mk2_cannon);snprintf(pc,sizeof(pc),"%d",d->plasma_cannon);
    snprintf(prc,sizeof(prc),"%d",d->proton_cannon);snprintf(ssd,sizeof(ssd),"%d",d->small_shield_dome);
    snprintf(lsd,sizeof(lsd),"%d",d->large_shield_dome);
    const char *params[10]={id,rl,ll,hl,ic,mk,pc,prc,ssd,lsd};
    PGresult *res=PQexecPrepared(r->db->conn,"di_upsert",10,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
