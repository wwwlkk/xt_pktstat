#include <stdbool.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <stddef.h>
#include <stdlib.h>
#include <getopt.h>
#include <iptables.h>
#include <linux/netfilter/xt_pktstat.h>

/* 
 * iptables -m pktstat -h 
 */
static void pktstat_mt_help(void)
{
   printf (
            "PKTSTAT v%s target options:\n"
            "    --period  <msecs>\t\tSampling period.\n"
            "    --samples <number>\t\tNumber of samples to buffer.\n"
            "    --name    <string>\t\tName to be given to the filter.\n",
         IPTABLES_VERSION);
}


static const struct option pktstat_mt_opts[] = {
    { .name = "samples", .has_arg = true, .val = 's' },
    { .name = "period",  .has_arg = true, .val = 'p' },
    { .name = "name",    .has_arg = true, .val = 'n' },
    { .name = NULL }
};

static void pktstat_mt_init(struct ipt_entry_match *match, unsigned int *nfcache)
{
    struct xt_pktstat_info* info = (struct xt_pktstat_info*)(match)->data;
    info->flags    = 0;
    info->samples  = 0;
    info->period   = 0;
    *info->name    = '\0';
}


static int pktstat_mt4_parse(int c, char **argv, int invert, unsigned int *flags,
                             const struct ipt_entry *entry,
                             unsigned int *nfcache,
                             struct ipt_entry_match **match)
{
    printf("c:%c optarg:%s\n", c, optarg);
    struct xt_pktstat_info* info = (struct xt_pktstat_info*)(*match)->data;

    switch (c) {
        /* --period */
        case 'p':
            info->period  = (uint64_t)atol(optarg) * 1000000L;
            *flags  |= PKTSTAT_PERIOD;
            break;
        /* --samples */
        case 's':
            info->samples = atoi(optarg);
            *flags  |= PKTSTAT_SAMPLES;
            break;
        /* --name */
        case 'n':
            strncpy(info->name, optarg, sizeof(info->name));
            info->name[sizeof(info->name)-1] = '\0';
            *flags  |= PKTSTAT_NAME;
            break;
        default:
            return false;
    }

    return true;
}

static void pktstat_mt_check(unsigned int flags)
{
    printf("check:start\n");
    if (!(flags & PKTSTAT_PERIOD)) {
        exit_error(PARAMETER_PROBLEM, "xt_pktstat: missing --period parameter");
    }
    if (!(flags & PKTSTAT_SAMPLES)) {
        exit_error(PARAMETER_PROBLEM, "xt_pktstat: missing --samples parameter");
    }
    if (!(flags & PKTSTAT_NAME)) {
        exit_error(PARAMETER_PROBLEM, "xt_pktstat: missing --name parameter");
    }
    printf("check:end\n");

}


static void pktstat_mt4_print(const struct ipt_ip *ip,
                              const struct ipt_entry_match *match, int numeric)
{
    const struct xt_pktstat_info *info = (const struct xt_pktstat_info *)match->data;

    if (info->flags & PKTSTAT_SAMPLES) {
        printf("samples %u ", info->samples);
    }

    if (info->flags & PKTSTAT_PERIOD) {
        printf("period %llu ", info->period);
    }

    if (info->flags & PKTSTAT_NAME) {
        printf("name %s ", info->name);
    }

}

/* 
 * iptables-saves
 */
static void pktstat_mt4_save(const struct ipt_ip *ip,
                             const struct ipt_entry_match *match)
{
    const struct xt_pktstat_info *info = (const struct xt_pktstat_info *)match->data;

    if (info->flags & PKTSTAT_PERIOD) {
        printf("--period %llu", info->period / 1000000);
    }

    if (info->flags & PKTSTAT_SAMPLES) {
        printf("--samples %u", info->samples);
    }

    if (info->flags & PKTSTAT_NAME) {
        printf("--name %s", info->name);
    }

}

static struct iptables_match pktstat_mt_reg
= {
    .version         = IPTABLES_VERSION,
    .name            = "pktstat",
    .revision        = 0,
    .size            = XT_ALIGN(sizeof(struct xt_pktstat_info)),
    .userspacesize   = offsetof(struct xt_pktstat_info, ctx),
    .help            = pktstat_mt_help,
    .init            = pktstat_mt_init,
    .parse           = pktstat_mt4_parse,
    .final_check     = pktstat_mt_check,
    .print           = pktstat_mt4_print,
    .save            = pktstat_mt4_save,
    .extra_opts      = pktstat_mt_opts
};

//void _init(void)
void libipt_pktstat_init(void)
{
    register_match(&pktstat_mt_reg);
}
