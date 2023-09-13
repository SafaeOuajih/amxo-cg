/****************************************************************************
**
** Copyright (c) 2020 SoftAtHome
**
** Redistribution and use in source and binary forms, with or
** without modification, are permitted provided that the following
** conditions are met:
**
** 1. Redistributions of source code must retain the above copyright
** notice, this list of conditions and the following disclaimer.
**
** 2. Redistributions in binary form must reproduce the above
** copyright notice, this list of conditions and the following
** disclaimer in the documentation and/or other materials provided
** with the distribution.
**
** Subject to the terms and conditions of this license, each
** copyright holder and contributor hereby grants to those receiving
** rights under this license a perpetual, worldwide, non-exclusive,
** no-charge, royalty-free, irrevocable (except for failure to
** satisfy the conditions of this license) patent license to make,
** have made, use, offer to sell, sell, import, and otherwise
** transfer this software, where such license applies only to those
** patent claims, already acquired or hereafter acquired, licensable
** by such copyright holder or contributor that are necessarily
** infringed by:
**
** (a) their Contribution(s) (the licensed copyrights of copyright
** holders and non-copyrightable additions of contributors, in
** source or binary form) alone; or
**
** (b) combination of their Contribution(s) with the work of
** authorship to which such Contribution(s) was added by such
** copyright holder or contributor, if, at the time the Contribution
** is added, such addition causes such combination to be necessarily
** infringed. The patent license shall not apply to any other
** combinations which include the Contribution.
**
** Except as expressly stated above, no rights or licenses from any
** copyright holder or contributor is granted under this license,
** whether expressly, by implication, estoppel or otherwise.
**
** DISCLAIMER
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
** CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
** INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
** MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
** DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
** CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF
** USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
** AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
** LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
** ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
** POSSIBILITY OF SUCH DAMAGE.
**
****************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <errno.h>
#include <ctype.h>

#include "utils.h"
#include "colors.h"
#include "version.h"

static FILE* output = NULL;
static FILE* output_param = NULL;
static FILE* output_header = NULL;

static char* header_file_bn = NULL;

static const char* file_header =
    ""
    "#include <stdlib.h>\n"
    "#include <stdio.h>\n"
    "\n"
    "#include <amxc/amxc.h>\n"
    "#include <amxp/amxp_signal.h>\n"
    "#include <amxd/amxd_dm.h>\n"
    "#include <amxd/amxd_object.h>\n"
    "\n";

static const char* func_start =
    "amxd_status_t _%s_%s(amxd_object_t* object,\n"
    "                     amxd_function_t* func,\n"
    "                     amxc_var_t* args,\n"
    "                     amxc_var_t* ret) {\n"
    "    amxd_status_t status = amxd_status_ok;\n"
    "    %s retval = %s;\n"
    "    \n";

static const char* func_any =
    "amxd_status_t _%s_%s(amxd_object_t* object,\n"
    "                     amxd_function_t* func,\n"
    "                     amxc_var_t* args,\n"
    "                     amxc_var_t* ret) {\n"
    "    amxd_status_t status = amxd_status_ok;\n"
    "    amxc_var_t retval;\n"
    "    \n";

static const char* void_func_start =
    "amxd_status_t _%s_%s(amxd_object_t* object,\n"
    "                     amxd_function_t* func,\n"
    "                     amxc_var_t* args,\n"
    "                     amxc_var_t* ret) {\n"
    "    amxd_status_t status = amxd_status_ok;\n"
    "    \n";

static const char* event_start =
    "void _%s(const char* const sig_name,\n"
    "         const amxc_var_t* const data,\n"
    "         void* const priv) {\n"
    "    \n";

static const char* header_file_start =
    "#if !defined(__DM_%s_H__)\n"
    "#define __DM_%s_H__\n"
    "\n"
    "#ifdef __cplusplus\n"
    "extern \"C\"\n"
    "{\n"
    "#endif\n"
    "\n";

static const char* header_file_end =
    "#ifdef __cplusplus\n"
    "}\n"
    "#endif\n"
    "\n"
    "#endif // __DM_%s_H__\n";

static const char* param_impl_get =
    "    const amxc_var_t* param = NULL;\n"
    "    param = amxd_object_get_param_value(object, \"%s\");\n"
    "    %s ret = amxd_object_get_value(%s, object, \"%s\", NULL);\n"
    "    return ret;\n"
    "    \n";

static const char* get_strict_arg =
    "    const %s %s = amxc_var_constcast(%s, GET_ARG(args, \"%s\"));\n";

static const char* get_complex_arg =
    "    amxc_var_t* %s = GET_ARG(args, \"%s\");\n";

static const char* get_arg =
    "    %s %s = amxc_var_dyncast(%s, GET_ARG(args, \"%s\"));\n";

static amxc_string_t end_func;

static void strip_odl(char* filename) {
    char* dot;
    if(filename) {
        dot = strrchr(filename, '.');
        if (dot != NULL && !strcmp(dot, ".odl")) {
            *dot = '\0';
        }
    }
}

static void get_prototype(const char* input, char** prototype) {
    char* bracket;
    if(input) {
        *prototype = strdup(input);
        bracket = strrchr(*prototype, '{');
	*(bracket-1) = '\0';
	strcat(*prototype, ";\n\n");
    }
}

static void convert_2_uppercase(char *str) {
    while (*str) {
        *str = toupper(*str);
        str++;
    }
}

static void ocg_dm_methods_start(amxo_parser_t* parser) {
    amxc_var_t* gens = amxo_parser_get_config(parser, "generators");
    amxc_var_t* var_dir_name = amxc_var_get_key(gens,
                                                "dm_methods",
                                                AMXC_VAR_FLAG_DEFAULT);
    const char* dir_name = amxc_var_constcast(cstring_t, var_dir_name);
    char* filename = parser->file == NULL ? strdup("out") : strdup(parser->file);
    char* bn = basename(filename);
    amxc_string_t file, file_param, header_file;
    amxc_string_init(&file, 0);
    amxc_string_init(&file_param, 0);
    amxc_string_init(&header_file, 0);

    // TODO: Better file name (strip odl extension - add xml extension)
    // If no file name from parser - generate file name ?
    // Parser can parse odl strings - then no file name is available
    strip_odl(filename);
    header_file_bn = strdup(bn);
    convert_2_uppercase(header_file_bn);
    if((dir_name == NULL) || (*dir_name == 0)) {
        amxc_string_setf(&file, "./%s.c", bn);
        amxc_string_setf(&file_param, "./%s_param.c", bn);
        amxc_string_setf(&header_file, "./dm_%s.h", bn);
    } else {
        amxc_string_setf(&file, "%s/%s.c", dir_name, bn);
        amxc_string_setf(&file_param, "%s/%s_param.c", dir_name, bn);
        amxc_string_setf(&header_file, "%s/dm_%s.h", dir_name, bn);
    }

    ocg_message(&parser->config, "Creating file [%s]", amxc_string_get(&file, 0));
    output = fopen(amxc_string_get(&file, 0), "w+");
    ocg_message(&parser->config, "Creating file [%s]", amxc_string_get(&file_param, 0));
    output_param = fopen(amxc_string_get(&file_param, 0), "w+");
    ocg_message(&parser->config, "Creating file [%s]", amxc_string_get(&header_file, 0));
    output_header = fopen(amxc_string_get(&header_file, 0), "w+");
    if(output == NULL) {
        ocg_error(&parser->config, "Error: Failed to create [%s]", amxc_string_get(&file, 0));
    } else {
        fprintf(output,
                "/* AUTO GENERATED WITH amx_odl_version %d.%d.%d */\n\n",
                VERSION_MAJOR,
                VERSION_MINOR,
                VERSION_BUILD);
        fprintf(output, "%s\n\n", file_header);
    }

    if(output_param == NULL) {
        ocg_error(&parser->config, "Error: Failed to create [%s]", amxc_string_get(&file_param, 0));
    } else {
        fprintf(output_param,
                "/* AUTO GENERATED WITH amx_odl_version %d.%d.%d */\n\n",
                VERSION_MAJOR,
                VERSION_MINOR,
                VERSION_BUILD);
        fprintf(output_param, "%s\n\n", file_header);
    }

    if(output_header == NULL) {
        ocg_error(&parser->config, "Error: Failed to create [%s]", amxc_string_get(&header_file, 0));
    } else {
        fprintf(output_header,
                "/* AUTO GENERATED WITH amx_odl_version %d.%d.%d */\n\n",
                VERSION_MAJOR,
                VERSION_MINOR,
                VERSION_BUILD);
        fprintf(output_header, header_file_start, header_file_bn, header_file_bn);
    }

    free(filename);
    amxc_string_clean(&header_file);
    amxc_string_clean(&file_param);
    amxc_string_clean(&file);
}

static void ocg_dm_methods_stop(UNUSED amxo_parser_t* parser) {
    if(output != NULL) {
        fclose(output);
    }
    if(output_param != NULL) {
        fclose(output_param);
    }
    if(output_header != NULL) {
        fprintf(output_header, header_file_end, header_file_bn);
	free(header_file_bn);
        fclose(output_header);
    }
}

static void ocg_dm_methods_add_event(const char* name) {
    char* prototype = NULL;

    fprintf(output, event_start, name);
    fprintf(output, "    /* < ADD IMPLEMENTATION HERE > */\n\n");
    fprintf(output, "}\n\n");
    get_prototype(event_start, &prototype);
    fprintf(output_header, prototype, name);
}

static void ocg_dm_methods_add_func(UNUSED amxo_parser_t* parser,
                                    amxd_object_t* object,
                                    const char* name,
                                    UNUSED int64_t attr_bitmask,
                                    uint32_t type) {
    const char* obj_name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);
    const char* type_name = amxc_var_get_type_name_from_id(type);
    char* prototype = NULL;

    if(output == NULL) {
        return;
    }

    amxc_string_init(&end_func, 0);

    switch(type) {
    case AMXC_VAR_ID_NULL:
        fprintf(output, void_func_start, obj_name, name);
        get_prototype(void_func_start, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    case AMXC_VAR_ID_CSTRING:
    case AMXC_VAR_ID_SSV_STRING:
    case AMXC_VAR_ID_CSV_STRING:
        type_name = "char*";
        fprintf(output, func_start, obj_name, name, type_name, "NULL");
        get_prototype(func_start, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    case AMXC_VAR_ID_INT8:
    case AMXC_VAR_ID_INT16:
    case AMXC_VAR_ID_INT32:
    case AMXC_VAR_ID_INT64:
    case AMXC_VAR_ID_UINT8:
    case AMXC_VAR_ID_UINT16:
    case AMXC_VAR_ID_UINT32:
    case AMXC_VAR_ID_UINT64:
    case AMXC_VAR_ID_FLOAT:
    case AMXC_VAR_ID_DOUBLE:
        fprintf(output, func_start, obj_name, name, type_name, "0");
        get_prototype(func_start, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    case AMXC_VAR_ID_BOOL:
        fprintf(output, func_start, obj_name, name, type_name, "false");
        get_prototype(func_start, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    case AMXC_VAR_ID_FD:
        fprintf(output, func_start, obj_name, name, type_name, "-1");
        get_prototype(func_start, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    case AMXC_VAR_ID_ANY:
        fprintf(output, func_any, obj_name, name);
        get_prototype(func_any, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    default:
        fprintf(output, func_start, obj_name, name, type_name, "NULL");
        get_prototype(func_start, &prototype);
        fprintf(output_header, prototype, obj_name, name);
        break;
    }
    free(prototype);
}

static void ocg_dm_methods_end_func(UNUSED amxo_parser_t* parser,
                                    UNUSED amxd_object_t* object,
                                    amxd_function_t* function) {
    uint32_t type = amxd_function_get_type(function);
    const char* type_name = amxc_var_get_type_name_from_id(type);
    if(output == NULL) {
        return;
    }

    fprintf(output, "\n");
    fprintf(output, "    /* < ADD IMPLEMENTATION HERE > */\n\n");

    switch(type) {
    case AMXC_VAR_ID_NULL:
        fprintf(output, "    amxc_var_clean(ret);\n\n");
        break;
    case AMXC_VAR_ID_ANY:
        break;
    default:
        fprintf(output, "    amxc_var_set(%s, ret, retval);\n\n", type_name);
        break;
    }

    if(!amxc_string_is_empty(&end_func)) {
        fprintf(output, "%s", amxc_string_get(&end_func, 0));
    }
    fprintf(output, "    return status;\n");
    fprintf(output, "}\n\n");

    amxc_string_clean(&end_func);
}

static void ocg_dm_methods_func_arg(UNUSED amxo_parser_t* parser,
                                    UNUSED amxd_object_t* object,
                                    UNUSED amxd_function_t* func,
                                    const char* name,
                                    int64_t attr_bitmask,
                                    uint32_t type,
                                    UNUSED amxc_var_t* def_value) {
    const char* type_name = amxc_var_get_type_name_from_id(type);
    const char* ctype_name = NULL;
    bool is_strict = IS_BIT_SET(attr_bitmask, amxd_aattr_strict);
    bool is_complex_type = false;
    if(output == NULL) {
        return;
    }

    switch(type) {
    case AMXC_VAR_ID_CSTRING:
        ctype_name = "char*";
        if(!is_strict) {
            amxc_string_appendf(&end_func, "    free(%s);\n", name);
        }
        break;
    case AMXC_VAR_ID_ANY:
    case AMXC_VAR_ID_HTABLE:
    case AMXC_VAR_ID_LIST:
        is_complex_type = true;
        break;
    case AMXC_VAR_ID_INT8:
    case AMXC_VAR_ID_INT16:
    case AMXC_VAR_ID_INT32:
    case AMXC_VAR_ID_INT64:
    case AMXC_VAR_ID_UINT8:
    case AMXC_VAR_ID_UINT16:
    case AMXC_VAR_ID_UINT32:
    case AMXC_VAR_ID_UINT64:
    case AMXC_VAR_ID_BOOL:
    case AMXC_VAR_ID_FLOAT:
    case AMXC_VAR_ID_DOUBLE:
        is_strict = false;
        break;
    default:
        ctype_name = type_name;
        break;
    }

    if(is_complex_type) {
        fprintf(output, get_complex_arg, name, name);
    } else if(is_strict) {
        fprintf(output, get_strict_arg, ctype_name, name, type_name, name);
    } else {
        fprintf(output, get_arg, ctype_name, name, type_name, name);
    }
}

static void get_type(uint32_t type, char** type_str)
{
    if(*type_str){
        return;
    }
    switch(type) {
    case AMXC_VAR_ID_CSTRING:
        *type_str = strdup("cstring_t");
        break;
    case AMXC_VAR_ID_INT8:
        *type_str = strdup("int8_t");
	break;
    case AMXC_VAR_ID_INT16:
        *type_str = strdup("int16_t");
        break;
    case AMXC_VAR_ID_INT32:
        *type_str = strdup("int32_t");
        break;
    case AMXC_VAR_ID_INT64:
        *type_str = strdup("int64_t");
	break;
    case AMXC_VAR_ID_UINT8:
        *type_str = strdup("uint8_t");
	break;
    case AMXC_VAR_ID_UINT16:
        *type_str = strdup("uint16_t");
        break;
    case AMXC_VAR_ID_UINT32:
        *type_str = strdup("uint32_t");
        break;
    case AMXC_VAR_ID_UINT64:
        *type_str = strdup("uint64_t");
        break;
    case AMXC_VAR_ID_BOOL:
        *type_str = strdup("bool");
        break;
    case AMXC_VAR_ID_FLOAT:
        *type_str = strdup("float");
        break;
    case AMXC_VAR_ID_DOUBLE:
        *type_str = strdup("double");
        break;
    default:
        *type_str = strdup("void");
        break;
    }
}

static void ocg_dm_param_set_get(UNUSED amxo_parser_t* parser,
                                 amxd_object_t* object,
                                 const char* name,
                                 UNUSED int64_t attr_bitmask,
                                 uint32_t type) {
    const char* obj_name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);
    printf("%s\n", obj_name);
    char* type_str = NULL;
    fprintf(output_param, event_start, name);
    get_type(type, &type_str);
    fprintf(output_param, param_impl_get, name, type_str, type_str, name);
    fprintf(output_param, "    /* < ADD IMPLEMENTATION HERE > */\n\n");
    fprintf(output_param, "}\n\n");
    free(type_str);
}

static amxo_hooks_t fgen_hooks = {
    .it = { .next = NULL, .prev = NULL, .llist = NULL },
    .comment = NULL,
    .start = ocg_dm_methods_start,
    .end = ocg_dm_methods_stop,
    .start_include = NULL,
    .end_include = NULL,
    .set_config = NULL,
    .start_section = NULL,
    .end_section = NULL,
    .create_object = NULL,
    .add_instance = NULL,
    .select_object = NULL,
    .end_object = NULL,
    .add_param = ocg_dm_param_set_get,
    .set_param = NULL,
    .end_param = NULL,
    .add_func = ocg_dm_methods_add_func,
    .add_func_arg = ocg_dm_methods_func_arg,
    .end_func = ocg_dm_methods_end_func,
    .add_mib = NULL,
    .set_counter = NULL,
    .add_event = ocg_dm_methods_add_event
};

void ocg_gen_dm_methods(amxo_parser_t* parser, bool enable) {
    if(enable) {
        amxo_parser_set_hooks(parser, &fgen_hooks);
    } else {
        amxo_parser_unset_hooks(parser, &fgen_hooks);
    }
}
