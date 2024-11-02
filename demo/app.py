from fastapi import FastAPI, Request, Body
from fastapi.responses import HTMLResponse
from starlette.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware

import rsa_py as rsa

app = FastAPI()

rsa_manager = rsa.RSA()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# Serve static files (like CSS) from the "static" directory
app.mount("/static", StaticFiles(directory="static"), name="static")

@app.get("/", response_class=HTMLResponse)
async def read_root(request: Request):
    return FileResponse("static/index.html")

@app.post("/api/rsa/generate-prime")
async def generate_keys(payload: dict = Body(...)):
    res = rsa_manager.generate_key_pair(int(payload["len"]))
    public_key, private_key = res[0], res[1]
    return {
        "p": private_key.p.to_string(),
        "q": private_key.q.to_string(),
        "n": private_key.n.to_string(),
        "d": private_key.d.to_string(),
        "e": public_key.e.to_string(),
    }

@app.post("/api/rsa/encrypt")
async def encrypt(payload: dict = Body(...)):
    value = rsa.BigInt("0x" + payload["message"].encode().hex())
    result = rsa_manager.encrypt(value)

    return {
        "cipher": result.to_string()
    }

def hex_to_string(h):
    # Remove "0x" prefix if present, then decode
    return bytes.fromhex(h[2:] if h.startswith("0x") else h).decode()

@app.post("/api/rsa/decrypt")
async def decrypt(payload: dict = Body(...)):
    value = rsa.BigInt(payload["cipher"])
    result = rsa_manager.decrypt(value)
    return {
        "message": hex_to_string(result.to_string())
    }
# @app.get("/greet", response_class=HTMLResponse)
# async def greet(request: Request, name: str = ""):
#     greeting = f"Hello, {name}!" if name else "Hello, stranger!"
#     return templates.TemplateResponse("index.html", {"request": request, "greeting": greeting})
