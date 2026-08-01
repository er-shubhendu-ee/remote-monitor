/**
 * @file      app_helper.c
 * @author:   Shubhendu B B
 * @date:     02/08/2026
 * @brief
 * @details   Distributed globally for free under the MIT License terms.
 *
 * @copyright Copyright (c) 2025 er-shubhendu-ee
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 **/

#include "app_helper.h"

int app_helper_test(void) { return 0; }

/**
 * @brief Convert app_server_Address_t to IP string (without port)
 *
 * @param buf Output buffer to store IP string
 * @param len Size of output buffer
 * @param addr Input address structure
 */
void app_helper_AddrToIpStr(char* buf, size_t len, const app_server_Address_t* addr) {
    if (!buf || len == 0 || !addr) {
        return;
    }

    if (addr->isIpV6) {
        // IPv6 (assume 16 bytes in addr->ip)
        snprintf(buf, len,
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x:"
                 "%02x%02x:%02x%02x:%02x%02x:%02x%02x",
                 addr->ip[0], addr->ip[1], addr->ip[2], addr->ip[3], addr->ip[4], addr->ip[5],
                 addr->ip[6], addr->ip[7], addr->ip[8], addr->ip[9], addr->ip[10], addr->ip[11],
                 addr->ip[12], addr->ip[13], addr->ip[14], addr->ip[15]);
    } else {
        // IPv4 (first 4 bytes of addr->ip)
        snprintf(buf, len, "%u.%u.%u.%u", addr->ip[0], addr->ip[1], addr->ip[2], addr->ip[3]);
    }
}

void app_helper_MgAddrToAuthorityStr(char* buf, size_t len, struct mg_addr* addr) {
    if (!addr) {
        snprintf(buf, len, "N/A");
        return;
    }

    if (!addr->is_ip6) {
        // IPv4: first 4 bytes of ip[]
        struct in_addr ip4;
        memcpy(&ip4.s_addr, addr->ip, 4);
        inet_ntop(AF_INET, &ip4, buf, len);
    } else {
        // IPv6: all 16 bytes
        struct in6_addr ip6;
        memcpy(&ip6.s6_addr, addr->ip, 16);
        inet_ntop(AF_INET6, &ip6, buf, len);
    }

    // Append port (convert network -> host order)
    size_t l = strlen(buf);
    snprintf(buf + l, len - l, ":%d", ntohs(addr->port));
}

/**
 * @brief Convert string "ip:port" or "[ipv6]:port" to address struct.
 * @param buf  Input buffer containing address string
 * @param len  Length of buffer (for safety)
 * @param addr Output struct to store parsed result
 */
void app_helper_AuthorityStrToAddr(char* buf, size_t len, app_server_Address_t* addr) {
    if (!buf || !addr || len == 0) {
        printf("Invalid arguments.\n");
        return;
    }

    memset(addr, 0, sizeof(*addr));

    // Ensure null termination
    char tmp[64];
    size_t copy_len = len < sizeof(tmp) - 1 ? len : sizeof(tmp) - 1;
    strncpy(tmp, buf, copy_len);
    tmp[copy_len] = '\0';

    char* ip_part = NULL;
    char* port_part = NULL;

    // Detect IPv6 format: [xxxx:xxxx::xxxx]:port
    if (tmp[0] == '[') {
        char* closing = strchr(tmp, ']');
        if (!closing) {
            printf("Invalid IPv6 format: missing ']'\n");
            return;
        }

        *closing = '\0';
        ip_part = tmp + 1;  // skip '['
        if (*(closing + 1) == ':') {
            port_part = closing + 2;
        }
        addr->isIpV6 = 1;
    } else {
        // IPv4 or hostname
        ip_part = strtok(tmp, ":");
        port_part = strtok(NULL, ":");
        addr->isIpV6 = 0;
    }

    // Validate IP
    if (!ip_part || strlen(ip_part) == 0) {
        printf("Invalid IP part.\n");
        return;
    }

    // Parse port if available
    if (port_part) {
        char* endptr = NULL;
        long port_val = strtol(port_part, &endptr, 10);
        if (*endptr != '\0' || port_val <= 0 || port_val > 65535) {
            printf("Invalid port: %s\n", port_part);
            return;
        }
        addr->port = (uint16_t)port_val;
    } else {
        addr->port = 80;  // default port
    }

    // Parse IP
    int af = addr->isIpV6 ? AF_INET6 : AF_INET;
    int res = inet_pton(af, ip_part, addr->ip);
    if (res != 1) {
        printf("Invalid IP address: %s\n", ip_part);
        return;
    }

    addr->ipV6Scope = 0;  // default, can be extended later

    // ✅ Success summary
    if (addr->isIpV6) {
        printf("Parsed IPv6 address with port %u\n", addr->port);
    } else {
        printf("Parsed IPv4 address %u.%u.%u.%u:%u\n", addr->ip[0], addr->ip[1], addr->ip[2],
               addr->ip[3], addr->port);
    }
}

/**
 * @brief Converts an app_server_Address_t to a string like "192.168.0.108:8080"
 *
 * @param buf Output buffer
 * @param len Buffer length
 * @param addr Pointer to address struct
 */
void app_helper_AddrToAuthorityStr(char* buf, size_t len, const app_server_Address_t* addr) {
    if (!buf || !addr || len == 0) {
        return;
    }

    buf[0] = '\0';  // ensure string is cleared

    char ipStr[INET6_ADDRSTRLEN] = {0};

    const void* src = addr->ip;
    const char* res = inet_ntop(addr->isIpV6 ? AF_INET6 : AF_INET, src, ipStr, sizeof(ipStr));

    if (res == NULL) {
        snprintf(buf, len, "<invalid-ip>");
        return;
    }

    if (addr->isIpV6) {
        // IPv6 authority format: [address]:port
        snprintf(buf, len, "[%s]:%u", ipStr, addr->port);
    } else {
        // IPv4 format: address:port
        snprintf(buf, len, "%s:%u", ipStr, addr->port);
    }
}