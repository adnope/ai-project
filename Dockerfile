FROM python:3.13.3-slim
WORKDIR /app
RUN apt-get update && apt-get install -y \
    build-essential \
    g++ \
    && rm -rf /var/lib/apt/lists/*
ENV PYTHONUNBUFFERED=1
COPY requirements.txt .
RUN pip install --upgrade pip
RUN pip install -r requirements.txt
COPY . .
RUN make build/main && rm -rf build/obj .dockerignore Dockerfile include Makefile requirements.txt src/*.cpp
RUN chmod +x build/*

CMD python src/python/app.py