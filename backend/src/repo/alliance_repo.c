#include "alliance_repo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
alliance_repo_t *alliance_repo_init(db_t *db){alliance_repo_t *r=calloc(1,sizeof(*r));if(r)r->db=db;return r;}
int alliance_repo_prepare(alliance_repo_t *r){
    if(!db_prepare(r->db,"al_create","INSERT INTO alliances(name,owner_id) VALUES($1,$2) RETURNING id"))return 0;
    if(!db_prepare(r->db,"al_find_id","SELECT id,name,owner_id,EXTRACT(EPOCH FROM created_at),bulletin FROM alliances WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"al_find_user","SELECT a.id,a.name,a.owner_id,EXTRACT(EPOCH FROM a.created_at),a.bulletin FROM alliances a JOIN alliance_members m ON a.id=m.alliance_id WHERE m.user_id=$1"))return 0;
    if(!db_prepare(r->db,"al_bulletin","UPDATE alliances SET bulletin=$2 WHERE id=$1"))return 0;
    if(!db_prepare(r->db,"al_delete","DELETE FROM alliances WHERE id=$1"))return 0;
    return 1;
}
int alliance_repo_create(alliance_repo_t *r,const char *name,entity_id_t owner_id,entity_id_t *out_id){
    char oid[32];snprintf(oid,sizeof(oid),"%lld",(long long)owner_id);
    const char *p[2]={name,oid};PGresult *res=PQexecPrepared(r->db->conn,"al_create",2,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK){PQclear(res);return 0;}
    *out_id=atoll(PQgetvalue(res,0,0));PQclear(res);return 1;
}
int alliance_repo_find_by_id(alliance_repo_t *r,entity_id_t id,alliance_t *out){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"al_find_id",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));strncpy(out->name,PQgetvalue(res,0,1),sizeof(out->name)-1);
    out->owner_id=atoll(PQgetvalue(res,0,2));out->created_at=(time_t)atol(PQgetvalue(res,0,3));
    strncpy(out->bulletin,PQgetvalue(res,0,4),sizeof(out->bulletin)-1);PQclear(res);return 1;
}
int alliance_repo_find_by_user(alliance_repo_t *r,entity_id_t uid,alliance_t *out){
    char id[32];snprintf(id,sizeof(id),"%lld",(long long)uid);
    const char *p[1]={id};PGresult *res=PQexecPrepared(r->db->conn,"al_find_user",1,p,NULL,NULL,0);
    if(PQresultStatus(res)!=PGRES_TUPLES_OK||PQntuples(res)==0){PQclear(res);return 0;}
    out->id=atoll(PQgetvalue(res,0,0));strncpy(out->name,PQgetvalue(res,0,1),sizeof(out->name)-1);
    out->owner_id=atoll(PQgetvalue(res,0,2));out->created_at=(time_t)atol(PQgetvalue(res,0,3));
    strncpy(out->bulletin,PQgetvalue(res,0,4),sizeof(out->bulletin)-1);PQclear(res);return 1;
}
int alliance_repo_update_bulletin(alliance_repo_t *r,entity_id_t id,const char *bulletin){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[2]={idstr,bulletin};PGresult *res=PQexecPrepared(r->db->conn,"al_bulletin",2,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
int alliance_repo_delete(alliance_repo_t *r,entity_id_t id){
    char idstr[32];snprintf(idstr,sizeof(idstr),"%lld",(long long)id);
    const char *p[1]={idstr};PGresult *res=PQexecPrepared(r->db->conn,"al_delete",1,p,NULL,NULL,0);
    int ok=PQresultStatus(res)==PGRES_COMMAND_OK;PQclear(res);return ok;
}
