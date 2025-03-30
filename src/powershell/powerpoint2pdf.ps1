# Specify the path to your PowerPoint file
$pptPath = "C:\Temp\sample123.pptx"

$tempDir = [System.IO.Path]::GetTempPath()

# Get the output PDF path by replacing the extension
$pdfPath = Join-Path $tempDir "sample123.pdf"

try {
    # Create PowerPoint COM object
    $powerpoint = New-Object -ComObject PowerPoint.Application
    
    # Open the presentation
    $presentation = $powerpoint.Presentations.Open($pptPath)
    
    # Save as PDF (32 represents the PDF format in PowerPoint)
    $presentation.SaveAs($pdfPath, 32)
    
    # Close the presentation
    $presentation.Close()
    
    # Quit PowerPoint
    $powerpoint.Quit()
    
    Write-Host "Conversion completed successfully. PDF saved to: $pdfPath"
}
catch {
    Write-Host "An error occurred: $_"
}
finally {
    # Clean up COM objects
    if ($presentation) {
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($presentation) | Out-Null
    }
    if ($powerpoint) {
        [System.Runtime.InteropServices.Marshal]::ReleaseComObject($powerpoint) | Out-Null
    }
}