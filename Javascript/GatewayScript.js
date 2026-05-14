function doPost(e) {  

  const data = JSON.parse(e.postData.contents);  

  

  const ss = SpreadsheetApp.getActiveSpreadsheet();  

   

  // === 1. 'gate system' sheet 

  const logSheet = ss.getSheetByName("gate system");  

  logSheet.appendRow([  

    new Date(),  

    data.action   // ENTRY or EXIT  

  ]);  

   

  // === 2. 'count' sheet 

  const countSheet = ss.getSheetByName("count"); 

   

  if (countSheet) { 

    let currentCount = countSheet.getRange("B2").getValue() || 0; 

     

    if (data.action === "ENTRY") { 

      currentCount++; 

    } else if (data.action === "EXIT") { 

      currentCount = Math.max(0, currentCount - 1); 

    } 

     

    const timestamp = new Date(); 

    countSheet.getRange("A2").setValue(timestamp); 

    countSheet.getRange("B2").setValue(currentCount); 

    countSheet.getRange("A2").setNumberFormat('dd/MM/yyyy HH:mm:ss'); 

  } 

  

  return ContentService.createTextOutput(JSON.stringify({ 

    status: "success", 

    count: currentCount 

  }));  

} 
