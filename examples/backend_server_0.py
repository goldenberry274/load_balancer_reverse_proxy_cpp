from http.server import HTTPServer, BaseHTTPRequestHandler
import time

class Handler(BaseHTTPRequestHandler):
    def do_GET(self):
        # Simulate a slow backend
        #time.sleep(2)

        self.send_response(200)
        self.end_headers()
        self.wfile.write(b"Hello from backend 1\n")

HTTPServer(("127.0.0.1", 9001), Handler).serve_forever()