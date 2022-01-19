function Get-Options {
    param ()
    
    # 引用C#的容器
    $options = [System.Collections.Generic.HashSet[string]]::new()

    # 只获取子文件夹名，用于Options
    Get-Childitem "./test_cploxplox/" -Exclude *.lox |

    ForEach-Object {
        $options.Add($_.Name)
    } | Out-Null

    return $options
}

function Invoke-Scripts {
    param (
        [string]$folder
    )
    if ($folder -eq 'ALL') {
        $sub_directory = [System.Collections.Generic.List[string]]@(Get-Options)
        $sub_directory | ForEach-Object {
            Invoke-Scripts($_)
        }
    }

    Get-ChildItem ('./test_cploxplox/' + $folder) -Filter *.lox |
    ForEach-Object {
        Write-Host("Current testfile: " + $_.BaseName)

        $proc = Start-Process notepad $_.FullName -PassThru
    
        .\cploxplox.exe -f $_.FullName
    
        Stop-Process $proc
    
        $continue = Read-Host "Press b(break) to stop testing"
        if ($continue -like 'B*') {
            break
        }
    }
    Read-Host
}

function Show-Menu {
    [CmdletBinding()]
    param ()

    # store the options in a List object for easy addition
    $list = [System.Collections.Generic.List[string]]@(Get-Options)
    $list.Add('ALL') | Out-Null

    # now start an endless loop for the menu handling
    while ($true) { 
        Clear-Host
        # loop through the options list and build the menu
        Write-Host "`r`nPlease choose a test-suite from the list below.`r`n"
        $index = 1
        $list | ForEach-Object { Write-Host ("{0}.`t{1}" -f $index++, $_ ) }
        Write-Host "Q.`tQuit"

        $selection = Read-Host "`r`nEnter Option"

        switch ($selection) {
            { $_ -like 'Q*' } { 
                # if the user presses 'Q', exit the function
                return 
            } 
            default {
                # test if a valid numeric input in range has been given
                if ([int]::TryParse($selection, [ref]$index)) {
                    if ($index -gt 0 -and $index -le $list.Count) {
                        # do whatever you need to perform
                        $selection = $list[$index - 1]  # this gives you the text of the selected item

                        Write-Host "Running test files under '$selection'" -ForegroundColor Green
                        
                        Invoke-Scripts $selection
                    }
                    else {
                        Write-Host "Please enter a valid option from the menu" -ForegroundColor Red
                    }
                }
                else {
                    Write-Host "Please enter a valid option from the menu" -ForegroundColor Red
                }
            }
        }

        # add a little pause and start over again
        Start-Sleep -Seconds 1
    }
}

# call the function
Show-Menu