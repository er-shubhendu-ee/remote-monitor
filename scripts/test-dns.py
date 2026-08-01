"""*
 * @file      test-dns.py
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
 *"""
# test-dns.py

import socket
import struct

HOSTNAME = "esp-server"
DNS_SERVER = "192.168.0.109"  # put your ESP32 DNS server IP
DNS_PORT = 53
HTTP_PORT = 80

# ---------------------------
# Build DNS query
# ---------------------------
def build_dns_query(name):
    header = struct.pack(">HHHHHH", 0x1234, 0x0100, 1, 0, 0, 0)  # ID, flags, QDCOUNT=1
    qname = b""
    for label in name.split('.'):
        qname += bytes([len(label)]) + label.encode()
    qname += b"\x00"  # end of name
    qtype_qclass = struct.pack(">HH", 1, 1)  # TYPE A, CLASS IN
    return header + qname + qtype_qclass

# ---------------------------
# Parse DNS response
# ---------------------------
def parse_dns_response(data):
    ancount = struct.unpack(">H", data[6:8])[0]
    if ancount == 0:
        return None
    # simple parser: last 4 bytes = IP of A record
    ip_bytes = data[-4:]
    return ".".join(str(b) for b in ip_bytes)

# ---------------------------
# Resolve hostname
# ---------------------------
def resolve_hostname(name):
    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    sock.settimeout(2)
    query = build_dns_query(name)
    sock.sendto(query, (DNS_SERVER, DNS_PORT))
    try:
        data, _ = sock.recvfrom(512)
        ip = parse_dns_response(data)
        return ip
    except socket.timeout:
        print("DNS query timed out")
        return None

# ---------------------------
# Make HTTP request
# ---------------------------
def http_request(ip):
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.settimeout(2)
    try:
        sock.connect((ip, HTTP_PORT))
        request = f"GET / HTTP/1.1\r\nHost: {HOSTNAME}\r\n\r\n"
        sock.send(request.encode())
        response = sock.recv(1024)
        print(response.decode())
    except Exception as e:
        print("HTTP request failed:", e)
    finally:
        sock.close()

# ---------------------------
# Run
# ---------------------------
resolved_ip = resolve_hostname(HOSTNAME)
if resolved_ip:
    print(f"{HOSTNAME} resolved to {resolved_ip}")
    http_request(resolved_ip)
else:
    print("Failed to resolve hostname")
