# PowerShell script to convert an Excel document to PDF
# Requires Microsoft Excel to be installed

param (
    [Parameter(Mandatory=$true)]
    [string]$ExcelFilePath,
    [Parameter(Mandatory=$true)]
    [string]$OutputPDFPath
)

try {
    # Validate input file exists
    if (-not (Test-Path $ExcelFilePath)) {
        throw "Input file does not exist: $ExcelFilePath"
    }

    # Create Excel application object
    $excel = New-Object -ComObject Excel.Application
    $excel.Visible = $false
    $excel.DisplayAlerts = $false

    # Open the workbook
    $workbook = $excel.Workbooks.Open($ExcelFilePath)

    # Save as PDF (0 is the xlTypePDF constant)
    $workbook.ExportAsFixedFormat(0, $OutputPDFPath)

    # Clean up
    $workbook.Close()
    $excel.Quit()

    Write-Output "Successfully converted $ExcelFilePath to $OutputPDFPath"
}
catch {
    Write-Error "Error converting file: $_"
}
finally {
    # Release COM objects
    if ($workbook) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($workbook) | Out-Null }
    if ($excel) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($excel) | Out-Null }
}