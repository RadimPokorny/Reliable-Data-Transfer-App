import subprocess
import time
import os
import hashlib
import socket
import random
import threading

BINARY = "./ipk-rdt"
SERVER_PORT = 5555
PROXY_PORT = 6666

class BiDirectionalProxy:
    def __init__(self, listen_port, target_addr, target_port, loss=0, delay=0, corrupt=0):
        self.listen_port = listen_port
        self.target_addr = target_addr
        self.target_port = target_port
        self.loss = loss / 100.0
        self.delay = delay / 1000.0
        self.corrupt = corrupt / 100.0
        self.running = False
        self.client_address = None

    def start(self):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.sock.bind(('127.0.0.1', self.listen_port))
        self.sock.settimeout(0.1)
        self.running = True

        def proxy_loop():
            while self.running:
                try:
                    data, addr = self.sock.recvfrom(2048)

                    if addr[1] != self.target_port:
                        self.client_address = addr
                        dest = (self.target_addr, self.target_port)
                        apply_faults = True
                    else:
                        if not self.client_address: continue
                        dest = self.client_address
                        apply_faults = False

                    if apply_faults and random.random() < self.loss: continue

                    if apply_faults and self.corrupt > 0 and random.random() < self.corrupt:
                        b_data = bytearray(data)
                        b_data[random.randint(0, len(b_data)-1)] ^= 0xFF
                        data = bytes(b_data)

                    def send_delayed(d, target):
                        if apply_faults and self.delay > 0: time.sleep(self.delay)
                        try: self.sock.sendto(d, target)
                        except: pass

                    threading.Thread(target=send_delayed, args=(data, dest), daemon=True).start()
                except: continue

        threading.Thread(target=proxy_loop, daemon=True).start()

    def stop(self):
        self.running = False
        time.sleep(0.2)
        self.sock.close()

def get_md5(filename):
    if not os.path.exists(filename) or os.path.getsize(filename) == 0:
        if os.path.exists(filename): return "empty_file"
        return None
    return hashlib.md5(open(filename, 'rb').read()).hexdigest()

def run_scenario(name, size_mb, loss=0, delay=0, corrupt=0):
    print(f"  [{name:12}] ", end="", flush=True)
    test_in, test_out = "test_in.bin", "test_out.bin"
    if os.path.exists(test_out): os.remove(test_out)

    with open(test_in, "wb") as f:
        f.write(os.urandom(int(size_mb * 1024 * 1024)) if size_mb > 0 else b"")

    proxy = BiDirectionalProxy(PROXY_PORT, "127.0.0.1", SERVER_PORT, loss, delay, corrupt)
    proxy.start()

    srv = subprocess.Popen([BINARY, "-s", "-p", str(SERVER_PORT), "-o", test_out], stderr=subprocess.PIPE)
    time.sleep(0.5)

    start_t = time.time()
    try:
        subprocess.run([BINARY, "-c", "-a", "127.0.0.1", "-p", str(PROXY_PORT), "-i", test_in],
                       timeout=15, stderr=subprocess.PIPE)
    except: pass

    time.sleep(1.0)
    if srv.poll() is None:
        srv.terminate()
        srv.wait()

    proxy.stop()

    in_md5 = get_md5(test_in)
    out_md5 = get_md5(test_out)
    passed = out_md5 is not None and in_md5 == out_md5

    status = "\033[92mPASS\033[0m" if passed else "\033[91mFAIL\033[0m"
    print(f"{status} ({time.time() - start_t:.1f}s)")
    return passed

if __name__ == "__main__":
    print("\n  IPK-RDT TEST HARNESS")
    print("  " + "─" * 40)
    results = [
        run_scenario("Clean", 0.1),
        run_scenario("Empty", 0),
        run_scenario("Loss 5%", 0.1, loss=5)
    ]
    print("  " + "─" * 40)
    print(f"  Result: {sum(results)}/{len(results)}\n")