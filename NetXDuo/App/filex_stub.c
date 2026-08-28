/**
 * @file    filex_stub.c
 * @brief   Stub implementations for the FileX services nx_web_http_server.c
 *          links against when NX_WEB_HTTP_NO_FILEX is defined (see
 *          filex_stub.h). This project has no FileX/media, and
 *          NetXDuo/App/app_netxduo.c's request_notify callback always
 *          returns NX_WEB_HTTP_CALLBACK_COMPLETED for every request, so
 *          the HTTP server's file-serving fallback code that calls these
 *          is dead code at runtime - these bodies only need to satisfy
 *          the linker and fail safely if that assumption is ever wrong.
 *
 * @author  Kemal UZGOREN
 * @date    2026-08-23
 *
 * SPDX-License-Identifier: MIT
 */

#include "tx_api.h"
#include "filex_stub.h"

UINT fx_directory_attributes_read(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes_ptr) {

    (void)media_ptr;
    (void)directory_name;
    (void)attributes_ptr;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_attributes_set(FX_MEDIA *media_ptr, CHAR *directory_name, UINT attributes) {

    (void)media_ptr;
    (void)directory_name;
    (void)attributes;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_create(FX_MEDIA *media_ptr, CHAR *directory_name) {

    (void)media_ptr;
    (void)directory_name;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_delete(FX_MEDIA *media_ptr, CHAR *directory_name) {

    (void)media_ptr;
    (void)directory_name;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_rename(FX_MEDIA *media_ptr, CHAR *old_directory_name, CHAR *new_directory_name) {

    (void)media_ptr;
    (void)old_directory_name;
    (void)new_directory_name;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_first_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name) {

    (void)media_ptr;
    (void)directory_name;
    return FX_NO_MORE_ENTRIES;
}

UINT fx_directory_first_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                         UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute,
                                         UINT *second) {

    (void)media_ptr;
    (void)directory_name;
    (void)attributes;
    (void)size;
    (void)year;
    (void)month;
    (void)day;
    (void)hour;
    (void)minute;
    (void)second;
    return FX_NO_MORE_ENTRIES;
}

UINT fx_directory_next_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name) {

    (void)media_ptr;
    (void)directory_name;
    return FX_NO_MORE_ENTRIES;
}

UINT fx_directory_next_full_entry_find(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                        UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute,
                                        UINT *second) {

    (void)media_ptr;
    (void)directory_name;
    (void)attributes;
    (void)size;
    (void)year;
    (void)month;
    (void)day;
    (void)hour;
    (void)minute;
    (void)second;
    return FX_NO_MORE_ENTRIES;
}

UINT fx_directory_name_test(FX_MEDIA *media_ptr, CHAR *directory_name) {

    (void)media_ptr;
    (void)directory_name;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_information_get(FX_MEDIA *media_ptr, CHAR *directory_name, UINT *attributes, ULONG *size,
                                   UINT *year, UINT *month, UINT *day, UINT *hour, UINT *minute, UINT *second) {

    (void)media_ptr;
    (void)directory_name;
    (void)attributes;
    (void)size;
    (void)year;
    (void)month;
    (void)day;
    (void)hour;
    (void)minute;
    (void)second;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_default_set(FX_MEDIA *media_ptr, CHAR *new_path_name) {

    (void)media_ptr;
    (void)new_path_name;
    return FX_ACCESS_ERROR;
}

UINT fx_directory_default_get(FX_MEDIA *media_ptr, CHAR **return_path_name) {

    (void)media_ptr;
    (void)return_path_name;
    return FX_ACCESS_ERROR;
}

UINT fx_file_best_effort_allocate(FX_FILE *file_ptr, ULONG size, ULONG *actual_size_allocated) {

    (void)file_ptr;
    (void)size;
    (void)actual_size_allocated;
    return FX_ACCESS_ERROR;
}

UINT fx_file_create(FX_MEDIA *media_ptr, CHAR *file_name) {

    (void)media_ptr;
    (void)file_name;
    return FX_ACCESS_ERROR;
}

UINT fx_file_delete(FX_MEDIA *media_ptr, CHAR *file_name) {

    (void)media_ptr;
    (void)file_name;
    return FX_ACCESS_ERROR;
}

UINT fx_file_rename(FX_MEDIA *media_ptr, CHAR *old_file_name, CHAR *new_file_name) {

    (void)media_ptr;
    (void)old_file_name;
    (void)new_file_name;
    return FX_ACCESS_ERROR;
}

UINT fx_file_attributes_set(FX_MEDIA *media_ptr, CHAR *file_name, UINT attributes) {

    (void)media_ptr;
    (void)file_name;
    (void)attributes;
    return FX_ACCESS_ERROR;
}

UINT fx_file_attributes_read(FX_MEDIA *media_ptr, CHAR *file_name, UINT *attributes_ptr) {

    (void)media_ptr;
    (void)file_name;
    (void)attributes_ptr;
    return FX_ACCESS_ERROR;
}

UINT fx_file_open(FX_MEDIA *media_ptr, FX_FILE *file_ptr, CHAR *file_name, UINT open_type) {

    (void)media_ptr;
    (void)file_ptr;
    (void)file_name;
    (void)open_type;
    return FX_ACCESS_ERROR;
}

UINT fx_file_close(FX_FILE *file_ptr) {

    (void)file_ptr;
    return FX_ACCESS_ERROR;
}

UINT fx_file_read(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG request_size, ULONG *actual_size) {

    (void)file_ptr;
    (void)buffer_ptr;
    (void)request_size;
    if (actual_size != FX_NULL) {
        *actual_size = 0;
    }
    return FX_END_OF_FILE;
}

UINT fx_file_write(FX_FILE *file_ptr, VOID *buffer_ptr, ULONG size) {

    (void)file_ptr;
    (void)buffer_ptr;
    (void)size;
    return FX_ACCESS_ERROR;
}

UINT fx_file_allocate(FX_FILE *file_ptr, ULONG size) {

    (void)file_ptr;
    (void)size;
    return FX_ACCESS_ERROR;
}

UINT fx_file_relative_seek(FX_FILE *file_ptr, ULONG byte_offset, UINT seek_from) {

    (void)file_ptr;
    (void)byte_offset;
    (void)seek_from;
    return FX_ACCESS_ERROR;
}

UINT fx_file_seek(FX_FILE *file_ptr, ULONG byte_offset) {

    (void)file_ptr;
    (void)byte_offset;
    return FX_ACCESS_ERROR;
}

UINT fx_file_truncate(FX_FILE *file_ptr, ULONG size) {

    (void)file_ptr;
    (void)size;
    return FX_ACCESS_ERROR;
}

UINT fx_file_truncate_release(FX_FILE *file_ptr, ULONG size) {

    (void)file_ptr;
    (void)size;
    return FX_ACCESS_ERROR;
}
