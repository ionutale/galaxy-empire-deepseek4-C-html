#include "wormhole_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
wormhole_repo_t *wormhole_repo_init(db_t *db){wormhole_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int wormhole_repo_prepare(wormhole_repo_t *r){
    if(!db_prepare(r->db,"wh_get","SELECT EXTRACT(EPOCH FROM cooldown_until) FROM wormhole_cooldowns WHERE user_id=$1 AND moon_a_id=$2 AND moon_b_id=$3"))return 0;
    if(!db_prepare(r->db,"wh_set","INSERT INTO wormhole_cooldowns(user_id,moon_a_id,moon_b_id,cooldown_until) VALUES($1,$2,$3,to_timestamp($4)) ON CONFLICT(user_id,moon_a_id,moon_b_id) DO UPDATE SET cooldown_until=to_timestamp($4)"))return 0;
    return 1;
}
int wormhole_repo_get_cooldown(wormhole_repo_t *r,entity_id_t uid,entity_id_t ma,entity_id_t mb,time_t *cd){
    char u[32],a[32],b[32]; snprintf(u,sizeof(u),"%lld",(long long)uid); snprintf(a,sizeof(a),"%lld",(long long)ma); snprintf(b,sizeof(b),"%lld",(long long)mb);
    const char *p[3]={u,a,b}; PGresult *res=PQexecPrepared(r->db->conn,"wh_get",3,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){*cd=0;PQclear(res);return 0;}
    *cd=(time_t)atol(PQgetvalue(res,0,0)); PQclear(res); return 1;
}
int wormhole_repo_set_cooldown(wormhole_repo_t *r,entity_id_t uid,entity_id_t ma,entity_id_t mb,time_t cd){
    char u[32],a[32],b[32],ts[16]; snprintf(u,sizeof(u),"%lld",(long long)uid); snprintf(a,sizeof(a),"%lld",(long long)ma); snprintf(b,sizeof(b),"%lld",(long long)mb);
    snprintf(ts,sizeof(ts),"%ld",(long)cd);
    const char *p[4]={u,a,b,ts}; PGresult *res=PQexecPrepared(r->db->conn,"wh_set",4,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK; PQclear(res); return ok;
}
