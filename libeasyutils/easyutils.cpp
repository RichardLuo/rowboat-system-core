/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * @author Richard Luo, cdominant7@gmail.com
 * @date   2011/10/22 09:45:32
 * 
 * @brief  
 * 
 ****************************************************************** 
 */

//#define LOG_NDEBUG 0
#define LOG_TAG "easyutils"
#include <cutils/easyutils.h>
#include <cutils/log.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <netdb.h>
#include <format.h>

/** 
 * @brief try the best to read @a size bytes from the fd
 * 
 * @param fd 
 * @param data 
 * @param size 
 * 
 * @return 1. OK: val equals @a size 2. Riched the END: 0 <= val < @a size 3. -1 on Error
 */
int read_n_l (int fd, void *data, size_t size) {
    int done_size = 0;
    const int total_size = size;
    uint8_t *pbuf = (uint8_t*)(data);
    while (done_size != total_size) {
        int once = read(fd, pbuf, total_size-done_size);
        LOG_IF_RETURN_CODE(once == 0, done_size,
                           "peer closed the socket, finished reading %d bytes", done_size);
        LOG_IF_RETURN(once < 0, "ERR: while reading socket:%d perror:%s", fd, strerror(errno));
        done_size += once;
        pbuf += once;
    }
    LOGFL("read_n with %d:%d bytes ok", done_size, size);
    return done_size;
}

static const size_t BYTES_PER_LINE = 16;

const uint8_t* hexdump_one_line(size_t line_no, const uint8_t *data, size_t data_size, std::string &res) {
    char line_buf[128];

    size_t posi = 0;
    posi += sprintf(&line_buf[posi], "%04x:\t", line_no * BYTES_PER_LINE);
    
    const size_t ds = data_size < BYTES_PER_LINE ? data_size : BYTES_PER_LINE;

    size_t i=0;
    for (; i < ds; ++i) {
        posi += sprintf(&line_buf[posi], "%02x ", data[i]);
        if ( (i+1) == BYTES_PER_LINE/2) {
            posi += sprintf(&line_buf[posi], "  ");
        }
    }

    for (; i < BYTES_PER_LINE; i++)
        posi += sprintf(&line_buf[posi], "    ");

    posi += sprintf(&line_buf[posi], "\t");

    for (i = 0; i < ds; i++)
        posi += sprintf(&line_buf[posi], "%c", (data[i] >= 0x20 && data[i] < 0x7f) ? data[i] : '.');

    posi += sprintf(&line_buf[posi], "\n");

    res += fmt::format("{:s}", line_buf);

    return &data[ds];
}


std::string hexdump_l(const void *data, int len) {
    const uint8_t *beg = (uint8_t*) data;
    const uint8_t *end = &beg[len];
    const uint8_t *pline = beg;
    
    size_t line = 0;
    size_t size = len;
    std::string res = "\n";
    while ((pline = hexdump_one_line(line, pline, size, res)) != end) {
        size = (end - pline);
        ++line;
    }
    res += "________________\n";

    return res;
}


void hexdump_l_org(const char *info, const void *data, int len)
{
    const uint8_t *pdata = (uint8_t*) data; 
    int thisline = 0, offset = 0, posi = 0;
    char *buf = (char*)malloc(4*len + 128);

    while (offset < len)
    {
        int i = 0;
        posi += sprintf(&buf[posi], "%04x:\t", offset);
        thisline = len - offset;
        if (thisline > 16)
            thisline = 16;

        for (i = 0; i < thisline; i++)
            posi += sprintf(&buf[posi], "%02x ", pdata[i]);

        for (; i < 16; i++)
            posi += sprintf(&buf[posi], "    ");

        posi += sprintf(&buf[posi], "\t");

        for (i = 0; i < thisline; i++)
            posi += sprintf(&buf[posi], "%c", (pdata[i] >= 0x20 && pdata[i] < 0x7f) ? pdata[i] : '.');

        posi += sprintf(&buf[posi], "\n");
        offset += thisline;
        pdata += thisline;
    }
    buf[posi] = 0;
    printf("\n@@@@ %s\n", info);
    printf("-------------------------------------------------------\n"
           "%s", buf);
    printf("--------------------- end of dump ---------------------\n\n");
    free(buf);
}
