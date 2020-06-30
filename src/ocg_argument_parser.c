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
#include <amxo/amxo.h>

#include "utils.h"

typedef void (*ocg_help_fn_t) (void);

typedef struct _help_topics {
    const char *topic;
    ocg_help_fn_t fn;
} help_topics_t;

static void ocg_usage(int argc, char *argv[]) {
    printf("%s [OPTIONS] <odl files>\n", argv[0]);

    printf("\n");
    printf("Options:\n");
    printf("--------\n");
    printf("\t-h   --help         Print this help\n");
    printf("\t-v   --verbose      Print verbose logging\n");
    printf("\t-I   --include-dir  Adds an include directory\n");
    printf("\t-L   --import-dir   Adds an import directory\n");
    printf("\t-G   --generator    Enables a generator (see generators)\n");
    printf("\n");
    printf("Generators:\n");
    printf("-----------\n");
    printf("\n");
    printf("\tGenerators can create files during the parsing of the odl file(s).\n\n");
    printf("\tThis %s supports:\n", argv[0]);
    printf("\t  - \"dm_methods\" use '--help dm_methods' for more information.\n");
    printf("\n");
}

static void ocg_usage_gen_dm_methods(void) {
    printf("\nData model methods C function template generator\n\n");
    printf("Option: -Gdm_methods,<output file>\n\n");
    printf("Description:\n");
    printf("------------\n\n");
    printf("\tThis generator generates a file containing with C functions.\n");
    printf("\tEach function in this file represents a data model object method.\n");
    printf("\n");
    printf("Arguments:\n");
    printf("----------\n");
    printf("\t<output_file> - mandatory - path (relative or absolute) and file name\n");
    printf("\n");
    printf("Example:\n");
    printf("--------\n");
    printf("\t-Gdm_methods,/tmp/my_dm_funcs.c\n\n");
}

static void ocg_sub_usage(const char *help_topic) {
    static help_topics_t topics[] = {
        { "dm_methods", ocg_usage_gen_dm_methods },
        { NULL, NULL }
    };

    for(int i = 0; topics[i].topic != NULL; i++) {
        if(strcmp(topics[i].topic, help_topic) == 0) {
            topics[i].fn();
        }
    }
}

static int add_generator(amxc_var_t *generators, char *generator) {
    int retval = 1;
    int i = 0;
    static const char *valids[] = {
        "dm_methods",
        NULL
    };

    for(i = 0; valids[i] != NULL; i++) {
        if(strncmp(generator, valids[i], strlen(valids[i])) == 0) {
            break;
        }
    }

    if(valids[i] == NULL) {
        fprintf(stderr, "Invalid generator [%s]\n", generator);
        goto exit;
    }

    if(generator[strlen(valids[i])] != ',') {
        fprintf(stderr, "Generator missing output file [%s]\n", generator);
        retval = 2;
        goto exit;
    }

    if(amxc_var_add_key(cstring_t,
                        generators,
                        valids[i],
                        generator + strlen(valids[i]) + 1) == NULL) {
        fprintf(stderr, "Duplicate generator specified [%s]\n", generator);
        retval = 3;
        goto exit;
    }

    retval = 0;

exit:
    return retval;
}

static void ocg_generators(amxo_parser_t *parser) {
    const amxc_htable_t *gens
        = amxc_var_constcast(amxc_htable_t,
                             amxo_parser_get_config(parser, "generators"));
    ocg_gen_dm_methods(parser, amxc_htable_contains(gens, "dm_methods"));
}

int ocg_parse_arguments(amxo_parser_t *parser,
                        amxc_var_t *config,
                        int argc,
                        char **argv) {
    int c;
    amxc_var_t *incdirs = NULL;
    amxc_var_t *libdirs = NULL;
    amxc_var_t *generators = NULL;
    amxc_var_set_type(config, AMXC_VAR_ID_HTABLE);

    while(1) {
        int option_index = 0;

        static struct option long_options[] = {
            {"help", optional_argument, 0, 'h' },
            {"verbose", no_argument, 0, 'v' },
            {"include-dir", required_argument, 0, 'I' },
            {"import-dir", required_argument, 0, 'L' },
            {"generator", required_argument, 0, 'G' },

            {0, 0, 0, 0 }
        };

        c = getopt_long(argc, argv, "h::vI:L:G:",
                        long_options, &option_index);
        if(c == -1) {
            break;
        }

        switch(c) {
        case 0:
            switch(option_index) {
            default:
                printf("Option index = %d\n", option_index);
            }
            break;
        case 'I':
            if(incdirs == NULL) {
                incdirs = amxc_var_add_key(amxc_llist_t,
                                           config,
                                           "include-dirs",
                                           NULL);
            }
            amxc_var_add(cstring_t, incdirs, optarg);
            break;

        case 'L':
            if(libdirs == NULL) {
                libdirs = amxc_var_add_key(amxc_llist_t,
                                           config,
                                           "import-dirs",
                                           NULL);
            }
            amxc_var_add(cstring_t, libdirs, optarg);
            break;

        case 'G':
            if(generators == NULL) {
                generators = amxc_var_add_key(amxc_htable_t,
                                              config,
                                              "generators",
                                              NULL);
            }
            if(add_generator(generators, optarg) != 0) {
                return -1;
            }
            break;

        case 'h':
            if((optarg == NULL) &&
               ( argv[optind] != NULL) &&
               ( argv[optind][0] != '-')) { // not an option
                optarg = argv[optind];
                optind++;
            } else {  // handle case of argument immediately after option
                if((optarg != NULL) && (optarg[0] == '=')) {
                    optarg++;
                }
            }
            if(optarg != NULL) {
                ocg_sub_usage(optarg);
            } else {
                ocg_usage(argc, argv);
            }
            return -1;
            break;

        case 'v':
            amxc_var_add_key(bool, config, "verbose", true);
            break;

        default:
            printf("?? getopt returned character code 0%o ??\n", c);
        }
    }

    return optind;
}

void ocg_config_changed(amxo_parser_t *parser, int section_id) {
    bool verbose = amxc_var_dyncast(bool,
                                    amxo_parser_get_config(parser, "verbose"));

    if(section_id != 0) {
        return;
    }

    ocg_verbose_logging(parser, verbose);
    ocg_generators(parser);
}

int ocg_apply_config(amxo_parser_t *parser,
                     amxc_var_t *config) {
    int retval = 0;
    const amxc_htable_t *options = amxc_var_constcast(amxc_htable_t, config);

    amxc_htable_for_each(it, options) {
        amxc_var_t *option = amxc_var_from_htable_it(it);
        const char *key = amxc_htable_it_get_key(it);
        retval = amxo_parser_set_config(parser, key, option);
        if(retval != 0) {
            goto exit;
        }
    }

exit:
    return retval;
}

