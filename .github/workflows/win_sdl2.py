# python3
from genericpath import exists
import requests
import os
from io import BytesIO  # Python 3
import zipfile

SDL2_URL = os.getenv('SDL2_URL')
SDL2_TTF_URL = os.getenv('SDL2_TTF_URL')
SDL2_PATH = os.getenv('SDL2_PATH')

if not os.path.exists(SDL2_PATH):
    os.makedirs(SDL2_PATH, exist_ok=True)
    
def get_and_extract(url: str) -> None:
    zip_response = requests.get(url, stream=True)
    zip = zipfile.ZipFile(BytesIO(zip_response.content))
    zip.extractall(SDL2_PATH)

get_and_extract(SDL2_URL)
get_and_extract(SDL2_TTF_URL)
