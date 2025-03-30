# Convert Word document to PDF using PowerShell

# Set the paths for input Word document and output PDF
$wordFile = "C:\Temp\tut-openboard.docx"
$pdfFile = "C:\Temp\tut-openboard.pdf"

# Create Word application object
$wordApp = New-Object -ComObject Word.Application

# Make Word invisible
$wordApp.Visible = $false

try {
    # Open the Word document
    $document = $wordApp.Documents.Open($wordFile)
    
    # Export as PDF (17 is the wdExportFormatPDF enumeration value)
    $document.ExportAsFixedFormat($pdfFile, 17)
    
    Write-Host "Conversion successful! PDF saved to: $pdfFile"
}
catch {
    Write-Host "An error occurred: $_"
}
finally {
    # Clean up
    if ($document) {
        $document.Close()
    }
    if ($wordApp) {
        $wordApp.Quit()
    }
    
    # Release COM objects
    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($document) | Out-Null
    [System.Runtime.InteropServices.Marshal]::ReleaseComObject($wordApp) | Out-Null
    Remove-Variable -Name document, wordApp
}