# Particle-System
A performant Particle System using ECS Architecture implemented with Vulkan


## Setup

### Windows
1. Clone the repository
```sh
git clone --recursive https://github.com/sebimih13/Particle-System
```

2. Change the current working directory
```sh
cd Particle-System
```

3. Generate the project
```sh
generate_project.bat
```

4. Compile and run
```sh
Particle-System.sln
```



### Linux

1. Clone the repository
```sh
git clone --recursive https://github.com/sebimih13/Particle-System
```

2. Change the current working directory
```sh
cd Particle-System
```

3. Generate the project
```sh
generate_project.sh
```

4. Compile
```sh
make config=debug
```
```sh
make config=release
```

5. Run
```sh
./bin/Debug-linux-x86_64/ParticleSystem/ParticleSystem
```
```sh
./bin/Release-linux-x86_64/ParticleSystem/ParticleSystem
```


## Requirements
### Windows
- Visual Studio including the *"Desktop development with C++"* workload

### Linux
- None


