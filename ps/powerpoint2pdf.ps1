# PowerShell script to convert a PowerPoint presentation to PDF
# Requires Microsoft PowerPoint to be installed

param (
    [Parameter(Mandatory=$true)]
    [string]$PowerPointFilePath,
    [Parameter(Mandatory=$true)]
    [string]$OutputPDFPath
)

try {
    # Validate input file exists
    if (-not (Test-Path $PowerPointFilePath)) {
        throw "Input file does not exist: $PowerPointFilePath"
    }

    # Create PowerPoint application object
    $powerpoint = New-Object -ComObject PowerPoint.Application

    # Get the Presentations collection
    $presentations = $powerpoint.Presentations

    # Open the presentation with additional parameters
    # Parameters: FileName, ReadOnly (msoFalse=0), Untitled (msoFalse=0), WithWindow (msoFalse=0)
    $msoFalse = 0
    $presentation = $presentations.Open($PowerPointFilePath, $msoFalse, $msoFalse, $msoFalse)

    if (-not $presentation) {
        throw "Failed to open presentation"
    }

    # Save as PDF (32 is the ppSaveAsPDF constant)
    $presentation.SaveAs($OutputPDFPath, 32)

    # Clean up
    $presentation.Close()
    $powerpoint.Quit()

    Write-Output "Successfully converted $PowerPointFilePath to $OutputPDFPath"
}
catch {
    Write-Error "Error converting file: $_"
}
finally {
    # Release COM objects
    if ($presentation) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($presentation) | Out-Null }
    if ($presentations) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($presentations) | Out-Null }
    if ($powerpoint) { [System.Runtime.InteropServices.Marshal]::ReleaseComObject($powerpoint) | Out-Null }
}