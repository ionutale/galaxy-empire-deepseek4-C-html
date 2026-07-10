#include "research_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
research_repo_t *research_repo_init(db_t *db) { research_repo_t *r = calloc(1,sizeof(research_repo_t)); if(r) r->db=db; return r; }
int research_repo_prepare(research_repo_t *r) {
    if(!db_prepare(r->db,"r_find","SELECT user_id,energy_tech,laser_tech,ion_tech,hyperspace_tech,plasma_tech,ultra_temperature_tech,anti_gravity_tech,weapons_tech,shielding_tech,strength_tech,combustion_drive,impulse_drive,hyperspace_drive,astrophysics_tech,computer_tech,espionage_tech,queued_tech,research_start,research_end FROM research WHERE user_id=$1")) return 0;
    if(!db_prepare(r->db,"r_upsert","INSERT INTO research(user_id,energy_tech,laser_tech,ion_tech,hyperspace_tech,plasma_tech,ultra_temperature_tech,anti_gravity_tech,weapons_tech,shielding_tech,strength_tech,combustion_drive,impulse_drive,hyperspace_drive,astrophysics_tech,computer_tech,espionage_tech) VALUES($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16,$17) ON CONFLICT(user_id) DO UPDATE SET energy_tech=EXCLUDED.energy_tech,laser_tech=EXCLUDED.laser_tech,ion_tech=EXCLUDED.ion_tech,hyperspace_tech=EXCLUDED.hyperspace_tech,plasma_tech=EXCLUDED.plasma_tech,ultra_temperature_tech=EXCLUDED.ultra_temperature_tech,anti_gravity_tech=EXCLUDED.anti_gravity_tech,weapons_tech=EXCLUDED.weapons_tech,shielding_tech=EXCLUDED.shielding_tech,strength_tech=EXCLUDED.strength_tech,combustion_drive=EXCLUDED.combustion_drive,impulse_drive=EXCLUDED.impulse_drive,hyperspace_drive=EXCLUDED.hyperspace_drive,astrophysics_tech=EXCLUDED.astrophysics_tech,computer_tech=EXCLUDED.computer_tech,espionage_tech=EXCLUDED.espionage_tech")) return 0;
    if(!db_prepare(r->db,"r_clearq","UPDATE research SET queued_tech=NULL,research_start=NULL,research_end=NULL WHERE user_id=$1")) return 0;
    return 1;
}
int research_repo_find_by_user(research_repo_t *r, entity_id_t user_id, research_t *out) {
    char id[32]; snprintf(id,sizeof(id),"%lld",(long long)user_id);
    const char *p[1]={id}; PGresult *res=PQexecPrepared(r->db->conn,"r_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    memset(out,0,sizeof(*out));
    out->user_id=atoll(PQgetvalue(res,0,0));
    out->energy_tech=atoi(PQgetvalue(res,0,1)); out->laser_tech=atoi(PQgetvalue(res,0,2));
    out->ion_tech=atoi(PQgetvalue(res,0,3)); out->hyperspace_tech=atoi(PQgetvalue(res,0,4));
    out->plasma_tech=atoi(PQgetvalue(res,0,5)); out->ultra_temperature_tech=atoi(PQgetvalue(res,0,6));
    out->anti_gravity_tech=atoi(PQgetvalue(res,0,7)); out->weapons_tech=atoi(PQgetvalue(res,0,8));
    out->shielding_tech=atoi(PQgetvalue(res,0,9)); out->strength_tech=atoi(PQgetvalue(res,0,10));
    out->combustion_drive=atoi(PQgetvalue(res,0,11)); out->impulse_drive=atoi(PQgetvalue(res,0,12));
    out->hyperspace_drive=atoi(PQgetvalue(res,0,13)); out->astrophysics=atoi(PQgetvalue(res,0,14));
    out->computer_tech=atoi(PQgetvalue(res,0,15)); out->espionage_tech=atoi(PQgetvalue(res,0,16));
    if(!PQgetisnull(res,0,17)) strncpy(out->queued_tech,PQgetvalue(res,0,17),sizeof(out->queued_tech)-1);
    if(!PQgetisnull(res,0,18)) out->research_start=atol(PQgetvalue(res,0,18));
    if(!PQgetisnull(res,0,19)) out->research_end=atol(PQgetvalue(res,0,19));
    PQclear(res); return 1;
}
int research_repo_upsert(research_repo_t *r, const research_t *rs) {
    char uid[32],e[16],l[16],io[16],h[16],p[16],ut[16],ag[16],w[16],sh[16],st[16],cd[16],idr[16],hd[16],as[16],c[16],es[16];
    snprintf(uid,sizeof(uid),"%lld",(long long)rs->user_id);
    snprintf(e,sizeof(e),"%d",rs->energy_tech); snprintf(l,sizeof(l),"%d",rs->laser_tech);
    snprintf(io,sizeof(io),"%d",rs->ion_tech); snprintf(h,sizeof(h),"%d",rs->hyperspace_tech);
    snprintf(p,sizeof(p),"%d",rs->plasma_tech); snprintf(ut,sizeof(ut),"%d",rs->ultra_temperature_tech);
    snprintf(ag,sizeof(ag),"%d",rs->anti_gravity_tech); snprintf(w,sizeof(w),"%d",rs->weapons_tech);
    snprintf(sh,sizeof(sh),"%d",rs->shielding_tech); snprintf(st,sizeof(st),"%d",rs->strength_tech);
    snprintf(cd,sizeof(cd),"%d",rs->combustion_drive); snprintf(idr,sizeof(idr),"%d",rs->impulse_drive);
    snprintf(hd,sizeof(hd),"%d",rs->hyperspace_drive); snprintf(as,sizeof(as),"%d",rs->astrophysics);
    snprintf(c,sizeof(c),"%d",rs->computer_tech); snprintf(es,sizeof(es),"%d",rs->espionage_tech);
    const char *params[17]={uid,e,l,io,h,p,ut,ag,w,sh,st,cd,idr,hd,as,c,es};
    PGresult *res=PQexecPrepared(r->db->conn,"r_upsert",17,params,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
void research_repo_clear_queue(research_repo_t *r, entity_id_t user_id) {
    char id[32]; snprintf(id,sizeof(id),"%lld",(long long)user_id);
    const char *p[1]={id}; PGresult *res=PQexecPrepared(r->db->conn,"r_clearq",1,p,NULL,NULL,0);
    PQclear(res);
}
