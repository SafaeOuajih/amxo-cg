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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>

#include <amxc/amxc.h>
#include <amxp/amxp_signal.h>
#include <amxd/amxd_dm.h>
#include <amxd/amxd_object.h>
#include <amxo/amxo.h>
#include <amxo/amxo_hooks.h>

#include "utils.h"
#include "colors.h"
#include "version.h"

static FILE* output = NULL;

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
    "amxd_status_t _%s_%s(amxd_object_t *object,\n"
    "                     amxd_function_t *func,\n"
    "                     amxc_var_t *args,\n"
    "                     amxc_var_t *ret) {\n"
    "    amxd_status_t status = amxd_status_ok;\n"
    "    %s retval;\n"
    "    \n";

static const char* get_strict_arg =
    "    const %s %s = amxc_var_constcast(%s, GET_ARG(args, \"%s\"));\n";

static const char* get_complex_arg =
    "    amxc_var_t *%s = GET_ARG(args, \"%s\");\n";

static const char* get_arg =
    "    %s %s = amxc_var_dyncast(%s, GET_ARG(args, \"%s\"));\n";

static amxc_string_t end_func;

static void ocg_gen_dm_methods_start(amxo_parser_t* parser) {
    amxc_var_t* gens = amxo_parser_get_config(parser, "generators");
    amxc_var_t* var_file_name = amxc_var_get_key(gens,
                                                 "dm_methods",
                                                 AMXC_VAR_FLAG_DEFAULT);
    const char* file_name = amxc_var_constcast(cstring_t, var_file_name);
    if(file_name == NULL) {
        return;
    }
    fprintf(stdout, "Creating file %s\n", file_name);
    fflush(stdout);
    output = fopen(file_name, "w+");
    if(output == NULL) {
        fprintf(stderr, "Error: Failed to create %s\n", file_name);
    } else {
        fprintf(output,
                "/* AUTO GENERATED WITH amx_odl_version %d.%d.%d */\n\n",
                VERSION_MAJOR,
                VERSION_MINOR,
                VERSION_BUILD);
        fprintf(output, "%s\n\n", file_header);
    }
}

static void ocg_gen_dm_methods_stop(amxo_parser_t* parser) {
    if(output != NULL) {
        fclose(output);
    }
}

static void ocg_gen_dm_methods_add_func(amxo_parser_t* parser,
                                        amxd_object_t* object,
                                        const char* name,
                                        int64_t attr_bitmask,
                                        uint32_t type) {
    const char* obj_name = amxd_object_get_name(object, AMXD_OBJECT_NAMED);
    const char* type_name = amxc_var_get_type_name_from_id(type);
    if(output == NULL) {
        return;
    }

    amxc_string_init(&end_func, 0);

    if(type == AMXC_VAR_ID_CSTRING) {
        type_name = "char *";
    }

    fprintf(output, func_start, obj_name, name, type_name);
}

static void ocg_gen_dm_methods_end_func(amxo_parser_t* parser,
                                        amxd_object_t* object,
                                        amxd_function_t* function) {
    uint32_t type = amxd_function_get_type(function);
    const char* type_name = amxc_var_get_type_name_from_id(type);
    if(output == NULL) {
        return;
    }
    fprintf(output, "\n");
    fprintf(output, "    /* < ADD IMPLEMENTATION HERE > */\n\n");
    fprintf(output, "    amxc_var_set(%s, ret, retval);\n\n", type_name);
    if(!amxc_string_is_empty(&end_func)) {
        fprintf(output, "%s", amxc_string_get(&end_func, 0));
    }
    fprintf(output, "    return status;\n");
    fprintf(output, "}\n\n");

    amxc_string_clean(&end_func);
}

static void ocg_gen_dm_methods_func_arg(amxo_parser_t* parser,
                                        amxd_object_t* object,
                                        amxd_function_t* func,
                                        const char* name,
                                        int64_t attr_bitmask,
                                        uint32_t type,
                                        amxc_var_t* def_value) {
    const char* type_name = amxc_var_get_type_name_from_id(type);
    const char* ctype_name = NULL;
    bool is_strict = IS_BIT_SET(attr_bitmask, amxd_aattr_strict);
    bool is_complex_type = false;
    if(output == NULL) {
        return;
    }

    switch(type) {
    case AMXC_VAR_ID_CSTRING:
        ctype_name = "char *";
        if(!is_strict) {
            amxc_string_appendf(&end_func, "    free(%s);\n", name);
        }
        break;
    case AMXC_VAR_ID_HTABLE:
        is_complex_type = true;
        break;
    case AMXC_VAR_ID_LIST:
        is_complex_type = true;
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

static amxo_hooks_t fgen_hooks = {
    .it = { .next = NULL, .prev = NULL, .llist = NULL },
    .start = ocg_gen_dm_methods_start,
    .end = ocg_gen_dm_methods_stop,
    .start_include = NULL,
    .end_include = NULL,
    .set_config = NULL,
    .start_section = NULL,
    .end_section = ocg_config_changed,
    .create_object = NULL,
    .add_instance = NULL,
    .select_object = NULL,
    .end_object = NULL,
    .add_param = NULL,
    .set_param = NULL,
    .end_param = NULL,
    .add_func = ocg_gen_dm_methods_add_func,
    .add_func_arg = ocg_gen_dm_methods_func_arg,
    .end_func = ocg_gen_dm_methods_end_func,
};

void ocg_gen_dm_methods(amxo_parser_t* parser, bool enable) {
    amxo_parser_set_hooks(parser, &fgen_hooks);
}