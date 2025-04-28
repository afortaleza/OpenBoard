# PowerShell script to convert a Word document to PDF
# Requires Microsoft Word to be installed

param (
    [Parameter(Mandatory=$true)]
    [string]$WordFilePath,
    [Parameter(Mandatory=$true)]
    [string]$OutputPDFPath
)

try {
    # Validate input file exists
    if (-not (Test-Path $WordFilePath)) {
        throw "Input file does not exist: $WordFilePath"
    }

    # Create Word application object
    $word = New-Object -ComObject Word.Application
    $word.Visible = $false

    # Open the document
    $doc = $word.Documents.Open($WordFilePath)

    # Save as PDF (17 is the wdFormatPDF constant)
    $doc.SaveAs([ref]$OutputPDFPath, [ref]17)

    # Clean up
    $doc.Close()
    $word.Quit()

    Write-Output "Successfully converted $WordFilePath to $OutputPDFPath"
}
catch {
    Write-Error "Error converting file: $_"
}
finally {
    # Release COM objects
    if ($doc) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($doc) | Out-Null }
    if ($word) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($word) | Out-Null }
}