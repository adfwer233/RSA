from fastapi import FastAPI, Request, Body
from fastapi.responses import HTMLResponse
from starlette.responses import FileResponse
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware

import rsa_py as rsa

app = FastAPI()

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
    r = rsa.RSA()

    prime_hex = r.generate_prime(192).to_string()

    return {
        "p": prime_hex
    }
# @app.get("/greet", response_class=HTMLResponse)
# async def greet(request: Request, name: str = ""):
#     greeting = f"Hello, {name}!" if name else "Hello, stranger!"
#     return templates.TemplateResponse("index.html", {"request": request, "greeting": greeting})
