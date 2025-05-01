FROM python:3.13.2-slim

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

RUN make

RUN chmod +x build/*

EXPOSE $PORT

# CMD uvicorn app:app --host 0.0.0.0 --port $PORT
CMD ["./build/main", "-w"]