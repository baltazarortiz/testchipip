
#include <vpi_user.h>
#include <svdpi.h>
#include <stdint.h>

#include "mm_dramsim2.h"

int dramsim = -1;

extern "C" void *memory_init(
        long long int mem_size,
        long long int word_size,
        long long int line_size,
        long long int id_bits)
{
    mm_t *mm;
    s_vpi_vlog_info info;
    std::string ini_dir = "dramsim2_ini";
    std::string loadmem;
    bool fastloadmem;

    if (dramsim < 0) {
        if (!vpi_get_vlog_info(&info))
            abort();

        dramsim = 0;
        for (int i = 1; i < info.argc; i++) {
            if (std::string(info.argv[i]).find("+dramsim") == 0)
                dramsim = 1;
            if (std::string(info.argv[i]).find("+fastloadmem") == 0)
                fastloadmem = true;
            if (std::string(info.argv[i]).find("+loadmem=") == 0)
                loadmem = info.argv[i] + strlen("+loadmem=");
            if (std::string(info.argv[i]).find("+dramsim_ini_dir=") == 0)
                ini_dir = info.argv[i] + strlen("+dramsim_ini_dir=");
        }
    }

    if (dramsim)
        mm = (mm_t *) (new mm_dramsim2_t(ini_dir, 1 << id_bits));
    else
        mm = (mm_t *) (new mm_magic_t);

    mm->init(mem_size, word_size, line_size);
    void * mem_data = mm->get_data();

    if (mm && fastloadmem && !loadmem.empty()) {
	    fprintf(stdout, "[fast loadmem] %s\n", loadmem.c_str());

	    // The load_mem function expects an array of pointers, one to each memory channel.
	    // For single channel, we just pass a double pointer to mem_data.
	    // The line_size argument is specified in bytes.
	    load_mem((void**)&mem_data, loadmem.c_str(), 64, 1);
    }

    return mm;
}

extern "C" void memory_tick(
        void *channel,

        unsigned char reset,

        unsigned char ar_valid,
        unsigned char *ar_ready,
        int ar_addr,
        int ar_id,
        int ar_size,
        int ar_len,

        unsigned char aw_valid,
        unsigned char *aw_ready,
        int aw_addr,
        int aw_id,
        int aw_size,
        int aw_len,

        unsigned char w_valid,
        unsigned char *w_ready,
        int w_strb,
        long long w_data,
        unsigned char w_last,

        unsigned char *r_valid,
        unsigned char r_ready,
        int *r_id,
        int *r_resp,
        long long *r_data,
        unsigned char *r_last,

        unsigned char *b_valid,
        unsigned char b_ready,
        int *b_id,
        int *b_resp)
{
    mm_t *mm = (mm_t *) channel;

    mm->tick(
        reset,

        ar_valid,
        ar_addr,
        ar_id,
        ar_size,
        ar_len,

        aw_valid,
        aw_addr,
        aw_id,
        aw_size,
        aw_len,

        w_valid,
        w_strb,
        &w_data,
        w_last,

        r_ready,
        b_ready);

    *ar_ready = mm->ar_ready();
    *aw_ready = mm->aw_ready();
    *w_ready = mm->w_ready();
    *r_valid = mm->r_valid();
    *r_id = mm->r_id();
    *r_resp = mm->r_resp();
    *r_data = *((long *) mm->r_data());
    *r_last = mm->r_last();
    *b_valid = mm->b_valid();
    *b_id = mm->b_id();
    *b_resp = mm->b_resp();
}
