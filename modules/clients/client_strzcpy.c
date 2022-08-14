/*
 * Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016 Nicira, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* Copies 'src' to 'dst'.  Reads no more than 'size - 1' bytes from 'src'.
 * Always null-terminates 'dst' (if 'size' is nonzero), and writes a zero byte
 * to every otherwise unused byte in 'dst'.
 *
 * Except for performance, the following call:
 *     client_strzcpy(dst, src, size);
 * is equivalent to these two calls:
 *     memset(dst, '\0', size);
 *     client_strlcpy(dst, src, size);
 *
 * (Thus, client_strzcpy() is similar to strncpy() without some of the pitfalls.)
 */

#include "client_strzcpy.h"

#include <string.h>

void client_strzcpy(char *dst, const char *src, size_t size)
{
    if (size > 0) {
        size_t len = strnlen(src, size - 1);
        memcpy(dst, src, len);
        memset(dst + len, '\0', size - len);
    }
}
