#include "fleet_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

fleet_repo_t *fleet_repo_init(db_t *db) { fleet_repo_t *r = calloc(1, sizeof(*r)); if(r) r->db = db; return r; }

static int scan_fleet(PGresult *res, int row, fleet_mission_t *m) {
    memset(m, 0, sizeof(*m));
    m->id = atoll(PQgetvalue(res, row, 0));
    m->owner_id = atoll(PQgetvalue(res, row, 1));
    m->origin_id = atoll(PQgetvalue(res, row, 2));
    m->target_galaxy = atoi(PQgetvalue(res, row, 3));
    m->target_system = atoi(PQgetvalue(res, row, 4));
    m->target_position = atoi(PQgetvalue(res, row, 5));
    m->target_is_moon = strcmp(PQgetvalue(res, row, 6), "t") == 0;
    m->mission_type = atoi(PQgetvalue(res, row, 7));
    m->departure_time = (time_t)atol(PQgetvalue(res, row, 8));
    m->arrival_time = (time_t)atol(PQgetvalue(res, row, 9));
    m->return_time = (time_t)atol(PQgetvalue(res, row, 10));
    m->ships.light_fighter = atoi(PQgetvalue(res, row, 11));
    m->ships.heavy_fighter = atoi(PQgetvalue(res, row, 12));
    m->ships.cruiser = atoi(PQgetvalue(res, row, 13));
    m->ships.battleship = atoi(PQgetvalue(res, row, 14));
    m->ships.dreadnought = atoi(PQgetvalue(res, row, 15));
    m->ships.destroyer = atoi(PQgetvalue(res, row, 16));
    m->ships.bomber = atoi(PQgetvalue(res, row, 17));
    m->ships.interceptor = atoi(PQgetvalue(res, row, 18));
    m->ships.iron_behemoth = atoi(PQgetvalue(res, row, 19));
    m->ships.devourer_interceptor = atoi(PQgetvalue(res, row, 20));
    m->ships.titanenergetic_frigate = atoi(PQgetvalue(res, row, 21));
    m->ships.annihilator = atoi(PQgetvalue(res, row, 22));
    m->ships.small_cargo = atoi(PQgetvalue(res, row, 23));
    m->ships.large_cargo = atoi(PQgetvalue(res, row, 24));
    m->ships.recycler = atoi(PQgetvalue(res, row, 25));
    m->ships.colony_ship = atoi(PQgetvalue(res, row, 26));
    m->ships.probe = atoi(PQgetvalue(res, row, 27));
    m->ships.solar_satellite = atoi(PQgetvalue(res, row, 28));
    m->recalled = strcmp(PQgetvalue(res, row, 29), "t") == 0;
    m->metal = atof(PQgetvalue(res, row, 30));
    m->crystal = atof(PQgetvalue(res, row, 31));
    m->gas = atof(PQgetvalue(res, row, 32));
    return 1;
}

int fleet_repo_prepare(fleet_repo_t *r) {
    const char *cols = "id,owner_id,origin_planet_id,target_galaxy,target_system,target_position,target_is_moon,mission_type,EXTRACT(EPOCH FROM departure_time),EXTRACT(EPOCH FROM arrival_time),EXTRACT(EPOCH FROM return_time),light_fighter,heavy_fighter,cruiser,battleship,dreadnought,destroyer,bomber,interceptor,iron_behemoth,devourer_interceptor,titanenergetic_frigate,annihilator,small_cargo,large_cargo,recycler,colony_ship,probe,solar_satellite,recalled,metal,crystal,gas";
    char q[2048];
    if (!db_prepare(r->db, "fl_create",
        "INSERT INTO fleet_missions(owner_id,origin_planet_id,target_galaxy,target_system,target_position,target_is_moon,mission_type,arrival_time,return_time,light_fighter,heavy_fighter,cruiser,battleship,dreadnought,destroyer,bomber,interceptor,iron_behemoth,devourer_interceptor,titanenergetic_frigate,annihilator,small_cargo,large_cargo,recycler,colony_ship,probe,solar_satellite,metal,crystal,gas) "
        "VALUES($1,$2,$3,$4,$5,$6,$7,to_timestamp($8),to_timestamp($9),$10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$20,$21,$22,$23,$24,$25,$26,$27,$28,$29,$30)")) return 0;
    snprintf(q, sizeof(q), "SELECT %s FROM fleet_missions WHERE arrival_time <= to_timestamp($1) AND recalled=false AND arrival_processed=false AND mission_type IN (1,2,3,4,5,6,7)", cols);
    if (!db_prepare(r->db, "fl_arrived", q)) return 0;
    snprintf(q, sizeof(q), "SELECT %s FROM fleet_missions WHERE owner_id=$1 AND recalled=false AND return_time>NOW() ORDER BY arrival_time", cols);
    if (!db_prepare(r->db, "fl_active", q)) return 0;
    snprintf(q, sizeof(q), "SELECT %s FROM fleet_missions WHERE id=$1", cols);
    if (!db_prepare(r->db, "fl_find", q)) return 0;
    if (!db_prepare(r->db, "fl_ret", "UPDATE fleet_missions SET return_time=to_timestamp($2) WHERE id=$1")) return 0;
    if (!db_prepare(r->db, "fl_recall", "UPDATE fleet_missions SET recalled=true WHERE id=$1")) return 0;
    if (!db_prepare(r->db, "fl_proc", "UPDATE fleet_missions SET arrival_processed=true WHERE id=$1")) return 0;
    if (!db_prepare(r->db, "fl_del", "DELETE FROM fleet_missions WHERE id=$1")) return 0;
    snprintf(q, sizeof(q), "UPDATE fleet_missions SET light_fighter=$2,heavy_fighter=$3,cruiser=$4,battleship=$5,dreadnought=$6,destroyer=$7,bomber=$8,interceptor=$9,iron_behemoth=$10,devourer_interceptor=$11,titanenergetic_frigate=$12,annihilator=$13,small_cargo=$14,large_cargo=$15,recycler=$16,colony_ship=$17,probe=$18,solar_satellite=$19 WHERE id=$1");
    if (!db_prepare(r->db, "fl_ships", q)) return 0;
    snprintf(q, sizeof(q), "UPDATE fleet_missions SET light_fighter=$2,heavy_fighter=$3,cruiser=$4,battleship=$5,dreadnought=$6,destroyer=$7,bomber=$8,interceptor=$9,iron_behemoth=$10,devourer_interceptor=$11,titanenergetic_frigate=$12,annihilator=$13,small_cargo=$14,large_cargo=$15,recycler=$16,colony_ship=$17,probe=$18,solar_satellite=$19,recalled=$20,metal=$21,crystal=$22,gas=$23,return_time=to_timestamp($24) WHERE id=$1");
    if (!db_prepare(r->db, "fl_update", q)) return 0;
    snprintf(q, sizeof(q), "SELECT %s FROM fleet_missions WHERE target_galaxy=$1 AND target_system=$2 AND target_position=$3 AND mission_type=7 AND recalled=false AND arrival_time<=to_timestamp($4) AND return_time>to_timestamp($4)", cols);
    if (!db_prepare(r->db, "fl_defense", q)) return 0;
    return 1;
}

int fleet_repo_create(fleet_repo_t *r, const fleet_mission_t *m, entity_id_t *out_id) {
    char oid[32],pid[32],g[8],sy[8],po[8],at[16],rt[16];
    snprintf(oid,sizeof(oid),"%lld",(long long)m->owner_id); snprintf(pid,sizeof(pid),"%lld",(long long)m->origin_id);
    snprintf(g,sizeof(g),"%d",m->target_galaxy); snprintf(sy,sizeof(sy),"%d",m->target_system); snprintf(po,sizeof(po),"%d",m->target_position);
    snprintf(at,sizeof(at),"%ld",(long)m->arrival_time); snprintf(rt,sizeof(rt),"%ld",(long)m->return_time);
    char im[4]; snprintf(im,sizeof(im),"%d",m->target_is_moon);
    char mt[8]; snprintf(mt,sizeof(mt),"%d",m->mission_type);
    char lf[16],hf[16],cr[16],bs[16],dr[16],de[16],bo[16],in[16],ib[16],di[16],tf[16],an[16],sc[16],lc[16],re[16],co[16],pr[16],ss[16];
    snprintf(lf,sizeof(lf),"%d",m->ships.light_fighter); snprintf(hf,sizeof(hf),"%d",m->ships.heavy_fighter);
    snprintf(cr,sizeof(cr),"%d",m->ships.cruiser); snprintf(bs,sizeof(bs),"%d",m->ships.battleship);
    snprintf(dr,sizeof(dr),"%d",m->ships.dreadnought); snprintf(de,sizeof(de),"%d",m->ships.destroyer);
    snprintf(bo,sizeof(bo),"%d",m->ships.bomber); snprintf(in,sizeof(in),"%d",m->ships.interceptor);
    snprintf(ib,sizeof(ib),"%d",m->ships.iron_behemoth); snprintf(di,sizeof(di),"%d",m->ships.devourer_interceptor);
    snprintf(tf,sizeof(tf),"%d",m->ships.titanenergetic_frigate); snprintf(an,sizeof(an),"%d",m->ships.annihilator);
    snprintf(sc,sizeof(sc),"%d",m->ships.small_cargo); snprintf(lc,sizeof(lc),"%d",m->ships.large_cargo);
    snprintf(re,sizeof(re),"%d",m->ships.recycler); snprintf(co,sizeof(co),"%d",m->ships.colony_ship);
    snprintf(pr,sizeof(pr),"%d",m->ships.probe); snprintf(ss,sizeof(ss),"%d",m->ships.solar_satellite);
    char me[32],crg[32],ga[32]; snprintf(me,sizeof(me),"%.0f",m->metal); snprintf(crg,sizeof(crg),"%.0f",m->crystal); snprintf(ga,sizeof(ga),"%.0f",m->gas);
    const char *p[30]={oid,pid,g,sy,po,im,mt,at,rt,lf,hf,cr,bs,dr,de,bo,in,ib,di,tf,an,sc,lc,re,co,pr,ss,me,crg,ga};
    PGresult *res=PQexecPrepared(r->db->conn,"fl_create",30,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_find_arrived(fleet_repo_t *r, time_t now, fleet_mission_t *out, int *cnt, int max) {
    char ts[16]; snprintf(ts,sizeof(ts),"%ld",(long)now);
    const char *p[1]={ts}; PGresult *res=PQexecPrepared(r->db->conn,"fl_arrived",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res); if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++) scan_fleet(res,i,&out[i]);
    PQclear(res); return 1;
}

int fleet_repo_find_active(fleet_repo_t *r, entity_id_t oid, fleet_mission_t *out, int *cnt, int max) {
    char id[32]; snprintf(id,sizeof(id),"%lld",(long long)oid);
    const char *p[1]={id}; PGresult *res=PQexecPrepared(r->db->conn,"fl_active",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res); if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++) scan_fleet(res,i,&out[i]);
    PQclear(res); return 1;
}

int fleet_repo_find_by_id(fleet_repo_t *r, entity_id_t id, fleet_mission_t *out) {
    char idstr[32]; snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr}; PGresult *res=PQexecPrepared(r->db->conn,"fl_find",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    scan_fleet(res,0,out); PQclear(res); return 1;
}

int fleet_repo_set_return_time(fleet_repo_t *r, entity_id_t id, time_t t) {
    char idstr[32],ts[16]; snprintf(idstr,sizeof(idstr),"%lld",(long long)id); snprintf(ts,sizeof(ts),"%ld",(long)t);
    const char *p[2]={idstr,ts}; PGresult *res=PQexecPrepared(r->db->conn,"fl_ret",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_mark_recalled(fleet_repo_t *r, entity_id_t id) {
    char idstr[32]; snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr}; PGresult *res=PQexecPrepared(r->db->conn,"fl_recall",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_mark_arrival_processed(fleet_repo_t *r, entity_id_t id) {
    char idstr[32]; snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr}; PGresult *res=PQexecPrepared(r->db->conn,"fl_proc",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_delete(fleet_repo_t *r, entity_id_t id) {
    char idstr[32]; snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr}; PGresult *res=PQexecPrepared(r->db->conn,"fl_del",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_update_ships(fleet_repo_t *r, entity_id_t id, const fleet_ships_t *s) {
    char idstr[32],lf[16],hf[16],cr[16],bs[16],dr[16],de[16],bo[16],in[16],ib[16],di[16],tf[16],an[16],sc[16],lc[16],re[16],co[16],pr[16],ss[16];
    snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    snprintf(lf,sizeof(lf),"%d",s->light_fighter); snprintf(hf,sizeof(hf),"%d",s->heavy_fighter);
    snprintf(cr,sizeof(cr),"%d",s->cruiser); snprintf(bs,sizeof(bs),"%d",s->battleship);
    snprintf(dr,sizeof(dr),"%d",s->dreadnought); snprintf(de,sizeof(de),"%d",s->destroyer);
    snprintf(bo,sizeof(bo),"%d",s->bomber); snprintf(in,sizeof(in),"%d",s->interceptor);
    snprintf(ib,sizeof(ib),"%d",s->iron_behemoth); snprintf(di,sizeof(di),"%d",s->devourer_interceptor);
    snprintf(tf,sizeof(tf),"%d",s->titanenergetic_frigate); snprintf(an,sizeof(an),"%d",s->annihilator);
    snprintf(sc,sizeof(sc),"%d",s->small_cargo); snprintf(lc,sizeof(lc),"%d",s->large_cargo);
    snprintf(re,sizeof(re),"%d",s->recycler); snprintf(co,sizeof(co),"%d",s->colony_ship);
    snprintf(pr,sizeof(pr),"%d",s->probe); snprintf(ss,sizeof(ss),"%d",s->solar_satellite);
    const char *p[19]={idstr,lf,hf,cr,bs,dr,de,bo,in,ib,di,tf,an,sc,lc,re,co,pr,ss};
    PGresult *res=PQexecPrepared(r->db->conn,"fl_ships",19,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_update(fleet_repo_t *r, const fleet_mission_t *m) {
    char idstr[32],lf[16],hf[16],cr[16],bs[16],dr[16],de[16],bo[16],in[16],ib[16],di[16],tf[16],an[16],sc[16],lc[16],re[16],co[16],pr[16],ss[16],rc[4],me[32],crg[32],ga[32],rt[16];
    snprintf(idstr,sizeof(idstr),"%lld",(long long)m->id);
    snprintf(lf,sizeof(lf),"%d",m->ships.light_fighter); snprintf(hf,sizeof(hf),"%d",m->ships.heavy_fighter);
    snprintf(cr,sizeof(cr),"%d",m->ships.cruiser); snprintf(bs,sizeof(bs),"%d",m->ships.battleship);
    snprintf(dr,sizeof(dr),"%d",m->ships.dreadnought); snprintf(de,sizeof(de),"%d",m->ships.destroyer);
    snprintf(bo,sizeof(bo),"%d",m->ships.bomber); snprintf(in,sizeof(in),"%d",m->ships.interceptor);
    snprintf(ib,sizeof(ib),"%d",m->ships.iron_behemoth); snprintf(di,sizeof(di),"%d",m->ships.devourer_interceptor);
    snprintf(tf,sizeof(tf),"%d",m->ships.titanenergetic_frigate); snprintf(an,sizeof(an),"%d",m->ships.annihilator);
    snprintf(sc,sizeof(sc),"%d",m->ships.small_cargo); snprintf(lc,sizeof(lc),"%d",m->ships.large_cargo);
    snprintf(re,sizeof(re),"%d",m->ships.recycler); snprintf(co,sizeof(co),"%d",m->ships.colony_ship);
    snprintf(pr,sizeof(pr),"%d",m->ships.probe); snprintf(ss,sizeof(ss),"%d",m->ships.solar_satellite);
    snprintf(rc,sizeof(rc),"%d",m->recalled); snprintf(me,sizeof(me),"%.0f",m->metal); snprintf(crg,sizeof(crg),"%.0f",m->crystal); snprintf(ga,sizeof(ga),"%.0f",m->gas);
    snprintf(rt,sizeof(rt),"%ld",(long)m->return_time);
    const char *p[24]={idstr,lf,hf,cr,bs,dr,de,bo,in,ib,di,tf,an,sc,lc,re,co,pr,ss,rc,me,crg,ga,rt};
    PGresult *res=PQexecPrepared(r->db->conn,"fl_update",24,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}

int fleet_repo_find_defense_at(fleet_repo_t *r, int galaxy, int system, int pos, time_t now, fleet_mission_t *out, int *cnt, int max) {
    char g[8],sy[8],po[8],ts[16]; snprintf(g,sizeof(g),"%d",galaxy); snprintf(sy,sizeof(sy),"%d",system); snprintf(po,sizeof(po),"%d",pos);
    snprintf(ts,sizeof(ts),"%ld",(long)now);
    const char *p[4]={g,sy,po,ts}; PGresult *res=PQexecPrepared(r->db->conn,"fl_defense",4,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){*cnt=0;PQclear(res);return 1;}
    *cnt=PQntuples(res); if(*cnt>max)*cnt=max;
    for(int i=0;i<*cnt;i++) scan_fleet(res,i,&out[i]);
    PQclear(res); return 1;
}
