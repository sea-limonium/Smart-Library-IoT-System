function doGet(e) { 

  const inventory = SpreadsheetApp.getActive().getSheetByName("Inventory"); 

  const logSheet = SpreadsheetApp.getActive().getSheetByName("Log"); 

 

  // Get parameters from URL 

  let uid = String(e.parameter.uid || "").replace(/\s+/g, '').toUpperCase().trim(); 

  const shelf = String(e.parameter.shelf || "").trim(); 

  const slot = String(e.parameter.slot || "").trim(); 

  const event = e.parameter.status || ""; 

 

  // Log event 

  logSheet.appendRow([new Date(), uid, shelf, slot, event]); 

 

  const COL_UID = 1; 

  const COL_EXP_SHELF = 6; 

  const COL_EXP_SLOT = 7; 

  const COL_CURR_SHELF = 8; 

  const COL_CURR_SLOT = 9; 

  const COL_STATUS = 10; 

 

  const inv = inventory.getDataRange().getValues(); 

 

  // CHECKED OUT 

  if (event === "CHECKED_OUT") { 

    let row = findByUID(inv, uid); 

    if (row === -1) return out("UID_NOT_FOUND"); 

 

    inventory.getRange(row, COL_STATUS).setValue("CHECKED_OUT"); 

 

    const bookUID = inv[row - 1][0]; 

    const bookName = inv[row - 1][1]; 

    const bookAuthor = inv[row - 1][2]; 

 

    return ContentService 

      .createTextOutput(JSON.stringify({ 

        status: "CHECKOUT_SUCCESS", 

        uid: bookUID, 

        bookName: bookName, 

        author: bookAuthor 

      })) 

      .setMimeType(ContentService.MimeType.JSON); 

  } 

 

  // INSERTED 

  if (event === "INSERTED") { 

    let row = findByUID(inv, uid); 

    if (row === -1) return out("UID_NOT_FOUND"); 

 

    const expShelf = String(inv[row - 1][COL_EXP_SHELF - 1]); 

    const expSlot = String(inv[row - 1][COL_EXP_SLOT - 1]); 

 

    inventory.getRange(row, COL_CURR_SHELF).setValue(shelf); 

    inventory.getRange(row, COL_CURR_SLOT).setValue(slot); 

 

    if (expShelf !== shelf || expSlot !== slot) { 

      inventory.getRange(row, COL_STATUS).setValue("MISMATCH"); 

      return out("MISMATCH_UPDATED"); 

    } 

 

    inventory.getRange(row, COL_STATUS).setValue("IN"); 

    return out("IN_UPDATED"); 

  } 

 

  // REMOVED 

  if (event === "REMOVED") { 

    let row = findByCurrentLocation(inv, shelf, slot); 

 

    if (row === -1 && uid !== "NONE") { 

      row = findByUID(inv, uid); 

    } 

 

    if (row === -1) return out("NOT_FOUND"); 

 

    inventory.getRange(row, COL_STATUS).setValue("OUT"); 

    inventory.getRange(row, COL_CURR_SHELF).setValue(""); 

    inventory.getRange(row, COL_CURR_SLOT).setValue(""); 

 

    return out("OUT_UPDATED"); 

  } 

 

  return out("UNKNOWN_EVENT"); 

} 

 

function doPost(e) { 

  const inventory = SpreadsheetApp.getActive().getSheetByName("Inventory"); 

  const logSheet = SpreadsheetApp.getActive().getSheetByName("Log"); 

 

  const data = JSON.parse(e.postData.contents); 

 

  let uid = String(data.uid).replace(/\s+/g, '').toUpperCase().trim(); 

  const shelf = String(data.shelf).trim(); 

  const slot = String(data.slot).trim(); 

  const event = data.status; 

 

  logSheet.appendRow([new Date(), uid, shelf, slot, event]); 

 

  const COL_UID = 1; 

  const COL_EXP_SHELF = 6; 

  const COL_EXP_SLOT = 7; 

  const COL_CURR_SHELF = 8; 

  const COL_CURR_SLOT = 9; 

  const COL_STATUS = 10; 

 

  const inv = inventory.getDataRange().getValues(); 

 

  // CHECKED OUT (for doPost) 

  if (event === "CHECKED_OUT") { 

    let row = findByUID(inv, uid); 

    if (row === -1) return out("UID_NOT_FOUND"); 

 

    inventory.getRange(row, COL_STATUS).setValue("CHECKED_OUT"); 

 

    const bookUID = inv[row - 1][0]; 

    const bookName = inv[row - 1][1]; 

    const bookAuthor = inv[row - 1][2]; 

 

    return ContentService 

      .createTextOutput(JSON.stringify({ 

        status: "CHECKOUT_SUCCESS", 

        uid: bookUID, 

        bookName: bookName, 

        author: bookAuthor 

      })) 

      .setMimeType(ContentService.MimeType.JSON); 

  } 

 

  // INSERTED (for doPost) 

  if (event === "INSERTED") { 

    let row = findByUID(inv, uid); 

    if (row === -1) return out("UID_NOT_FOUND"); 

 

    const expShelf = String(inv[row - 1][COL_EXP_SHELF - 1]); 

    const expSlot = String(inv[row - 1][COL_EXP_SLOT - 1]); 

 

    inventory.getRange(row, COL_CURR_SHELF).setValue(shelf); 

    inventory.getRange(row, COL_CURR_SLOT).setValue(slot); 

 

    if (expShelf !== shelf || expSlot !== slot) { 

      inventory.getRange(row, COL_STATUS).setValue("MISMATCH"); 

      return out("MISMATCH_UPDATED"); 

    } 

 

    inventory.getRange(row, COL_STATUS).setValue("IN"); 

    return out("IN_UPDATED"); 

  } 

 

  // REMOVED (for doPost) 

  if (event === "REMOVED") { 

    let row = findByCurrentLocation(inv, shelf, slot); 

 

    if (row === -1 && uid !== "NONE") { 

      row = findByUID(inv, uid); 

    } 

 

    if (row === -1) return out("NOT_FOUND"); 

 

    inventory.getRange(row, COL_STATUS).setValue("OUT"); 

    inventory.getRange(row, COL_CURR_SHELF).setValue(""); 

    inventory.getRange(row, COL_CURR_SLOT).setValue(""); 

 

    return out("OUT_UPDATED"); 

  } 

 

  return out("UNKNOWN_EVENT"); 

} 

 

function findByUID(inv, uid) { 

  uid = uid.toUpperCase().trim(); 

  for (let i = 1; i < inv.length; i++) { 

    let cleaned = String(inv[i][0]).replace(/\s+/g, '').toUpperCase().trim(); 

    if (cleaned === uid) return i + 1; 

  } 

  return -1; 

} 

 

function findByCurrentLocation(inv, shelf, slot) { 

  for (let i = 1; i < inv.length; i++) { 

    if (String(inv[i][7]) === shelf && String(inv[i][8]) === slot) { 

      return i + 1; 

    } 

  } 

  return -1; 

} 

 

function out(msg) { 

  return ContentService 

    .createTextOutput(JSON.stringify({ status: msg })) 

    .setMimeType(ContentService.MimeType.JSON); 

} 
