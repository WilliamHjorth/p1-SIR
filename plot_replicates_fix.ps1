$st = Get-Content -Raw -Path 'stochastic_replicates.txt' -ErrorAction SilentlyContinue
if (-not $st) { Write-Host 'stochastic_replicates.txt not found; aborting'; exit 1 }
$num = ($st -split '# Replicate').Count - 1
Write-Host "Found $num replicates"
$old = Get-Content -Raw -Path 'plot_replicates.gnu' -ErrorAction SilentlyContinue
if (-not $old) { Write-Host 'plot_replicates.gnu not found; aborting'; exit 1 }
$pattern = [regex]"for \[i=0:(\d+)\] '([^']+)' index i using 1:(\d+) with lines ls (\d+) title '([^']*)'"
$new = $old
$matches = $pattern.Matches($old)
foreach ($m in $matches) {
    $file = $m.Groups[2].Value
    $col = [int]$m.Groups[3].Value
    $ls = $m.Groups[4].Value
    $label = $m.Groups[5].Value
    $parts = @()
    for ($rep=0; $rep -lt $num; $rep++) {
        if ($rep -eq 0) { $parts += "'$file' index $rep using 1:$col with lines ls $ls title '$label'" }
        else { $parts += "'$file' index $rep using 1:$col with lines ls $ls title ''" }
    }
    $replacement = ($parts -join ", \\`n")
    $new = $new -replace [regex]::Escape($m.Value), [regex]::Escape($replacement)
    # After replace above, unescape replacement back
    $new = $new -replace [regex]::Escape($replacement), $replacement
}
$new = $new -replace "\nreplot ", "\n"
Set-Content -Path 'plot_replicates_fixed.gnu' -Value $new
Write-Host 'Wrote plot_replicates_fixed.gnu'