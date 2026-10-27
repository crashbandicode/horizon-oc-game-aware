$ErrorActionPreference = 'Stop'
$binary = Join-Path ([IO.Path]::GetTempPath()) ('hoc-game-aware-tests-' + $PID)
try {
    & /usr/bin/g++ -std=c++20 -Wall -Wextra -Werror "-I$PSScriptRoot/shim" "-I$PSScriptRoot/../common/include" "$PSScriptRoot/game_aware_policy.cpp" -o $binary
    if ($LASTEXITCODE -ne 0) { throw "Host test compilation failed: $LASTEXITCODE" }
    & $binary
    if ($LASTEXITCODE -ne 0) { throw "Host tests failed: $LASTEXITCODE" }
} finally {
    if (Test-Path $binary) { Remove-Item $binary }
}
