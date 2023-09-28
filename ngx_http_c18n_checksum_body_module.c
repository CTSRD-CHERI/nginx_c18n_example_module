#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>

#include <stdint.h>
#include <zlib.h>

typedef struct {
    ngx_flag_t         enable;
} ngx_http_c18n_checksum_body_conf_t;

static void *ngx_http_c18n_checksum_body_create_conf(ngx_conf_t *cf);
static char *ngx_http_c18n_checksum_body_merge_conf(ngx_conf_t *cf, void *parent,
    void *child);
static ngx_int_t ngx_http_c18n_checksum_body_init(ngx_conf_t *cf);

static ngx_command_t ngx_http_c18n_checksum_body_commands[] = {
  { ngx_string("checksum_body"),
    NGX_HTTP_MAIN_CONF|NGX_HTTP_SRV_CONF|NGX_HTTP_LOC_CONF|NGX_CONF_FLAG,
    ngx_conf_set_flag_slot,
    NGX_HTTP_LOC_CONF_OFFSET,
    offsetof(ngx_http_c18n_checksum_body_conf_t, enable),
    NULL },

    ngx_null_command
};

static ngx_http_module_t ngx_http_c18n_checksum_body_module_ctx = {
  NULL,
  ngx_http_c18n_checksum_body_init,
  NULL,
  NULL,
  NULL,
  NULL,
  ngx_http_c18n_checksum_body_create_conf,
  ngx_http_c18n_checksum_body_merge_conf
};

ngx_module_t ngx_http_c18n_checksum_body_module = {
  NGX_MODULE_V1,
  &ngx_http_c18n_checksum_body_module_ctx,
  ngx_http_c18n_checksum_body_commands,
  NGX_HTTP_MODULE,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NGX_MODULE_V1_PADDING
};

static ngx_http_request_body_filter_pt ngx_http_next_request_body_filter;
//static ngx_http_request_body_filter_pt ngx_http_next_body_filter;

static ngx_int_t ngx_http_c18n_checksum_body_filter(ngx_http_request_t *r, ngx_chain_t *in)
{
  u_char *p;
  ngx_chain_t *cl;
  ngx_http_c18n_checksum_body_conf_t *conf;

  conf = ngx_http_get_module_loc_conf(r, ngx_http_c18n_checksum_body_module);
  if (!conf->enable) {
    ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                  "checksum body filter disabled");
    return ngx_http_next_request_body_filter(r, in);
  }

  ngx_log_debug0(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                 "checksum request body filter start");

  uint32_t crc = 0;
  size_t len = 0;
  crc = crc32_z(0L, Z_NULL, 0);
  for (cl = in; cl != NULL; cl = cl->next) {
    p = cl->buf->pos;
    len = cl->buf->last - cl->buf->pos;
    if (p != NULL && len > 0) { 
	crc = crc32_z(crc, p, len);
     }
  }
  ngx_log_debug1(NGX_LOG_DEBUG_HTTP, r->connection->log, 0,
                 "checksum request body filter end crc = %08uXD", crc);

  return ngx_http_next_request_body_filter(r, in);
}

static void *
ngx_http_c18n_checksum_body_create_conf(ngx_conf_t *cf)
{
    ngx_http_c18n_checksum_body_conf_t  *conf;

    conf = ngx_pcalloc(cf->pool, sizeof(ngx_http_c18n_checksum_body_conf_t));
    if (conf == NULL) {
        return NULL;
    }

    conf->enable = NGX_CONF_UNSET;

    return conf;
}

static char *
ngx_http_c18n_checksum_body_merge_conf(ngx_conf_t *cf, void *parent, void *child)
{
    ngx_http_c18n_checksum_body_conf_t *prev = parent;
    ngx_http_c18n_checksum_body_conf_t *conf = child;

    ngx_conf_merge_value(conf->enable, prev->enable, 0);

    return NGX_CONF_OK;
}

static ngx_int_t ngx_http_c18n_checksum_body_init(ngx_conf_t *cf)
{
  ngx_http_next_request_body_filter = ngx_http_top_request_body_filter;
  ngx_http_top_request_body_filter = ngx_http_c18n_checksum_body_filter;

  return NGX_OK;
}

