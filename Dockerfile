FROM ubuntu:22.04

# Instalacija svih zavisnosti
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    qtbase5-dev \
    qt5-qmake \
    libqt5widgets5 \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libglm-dev \
    libassimp-dev \
    libxinerama-dev \
    libxi-dev \
    libxxf86vm-dev \
    libxcursor-dev \
    libfreetype6-dev \
    && apt-get clean

# Radni direktorijum
WORKDIR /app

# Kopiraj ceo projekat
COPY . .

# Build
RUN mkdir build && cd build && cmake .. && make

# Postavi build folder kao trenutni
WORKDIR /app

# Pokreni aplikaciju
CMD ["./project_base"]

