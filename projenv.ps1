# Source this env to quickly build, clear and run by these aliases
# Run: . .\projenv.ps1

function build {
    param(
        [string]$Target
    )

    if (-not (Test-Path -Path build -PathType Container)) {
        New-Item -ItemType Directory -Name build | Out-Null
        cmake -B build .
    }

    if ($Target) {
        cmake --build build --target $Target
    }
    else {
        clean
        cmake --build build
    }
}

function clean {
    if (-not (Test-Path -Path build -PathType Container)) {
        Write-Error "Nothing to clean: 'build' directory missing."
    }
    else {
        cmake --build build --target clean
    }
}

function main {
    $exePath = "build\main.exe"
    if (-not (Test-Path $exePath)) {
        Write-Error "Error: '$exePath' not found. Build first!"
    }
    else {
        & $exePath @args
    }
}

function generator {
    $exePath = "build\generator.exe"
    if (-not (Test-Path $exePath)) {
        Write-Error "Error: '$exePath' not found. Build first!"
    }
    else {
        & $exePath @args
    }
}