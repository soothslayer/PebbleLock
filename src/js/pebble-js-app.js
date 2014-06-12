var initialized = false;
//lockitron variables
var lockitronUrl = 'https://api.lockitron.com/v1/locks';
var lockIndex = 0;
var lockId = '';
var lockNames = [];
var lockIDs = [];
var lockCounts = 0;
var messageBlock = [];
var readyState = false;
//load access_token from local Storage
var accessToken = localStorage.getItem("accessToken");				

//funtion to send a message to the pebble that checks if the line is clear
var sendMessage = function(dictionary) {
	for (var key in dictionary) {
		console.log("Key " + key + " in " + dictionary[key]);
	}
	console.log("messageBlock length was " + messageBlock.length);
	//add the dictionary to the message block
	messageBlock.push(dictionary);
	console.log("now before sending the massage it is" + messageBlock.length);
	//if the pebble is ready to accept messages then send a message
	//if (readyState) {
	if (readyState) {
	readyState = false;
		Pebble.sendAppMessage(messageBlock[0],
			function(e) {
				console.log("1. Successfully delivered message with transactionId=" + e.data.transactionId);
				readyState = true;
				if (messageBlock.length !== 0) {
					messageBlock.shift();
					sendMessage(messageBlock[0]);
				}
			},
			function(e) {
				//readyState = true;
				sendMessage(messageBlock[0]);
				console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e.error.message);
			}
		);
	}
		//and change the state back to not ready;
		//readyState = false;
	//otherwise add it to the message block to be sent when the pebble is ready (see eventlistener app message)
	//} else {
					////console.log("adding message:" + dictionary.lockName + "to log");
		//messageBlock.push(dictionary);
	//}			
};
					
var updateLockIndex = function() {
  for ( var i = 0, ii = lockNames.length; i < ii; ++i ) {
    if (lockIDs[i].id === lockId) {
      lockIndex = i;
      return;
   }
  }
};

//update the list of locks and send it to the Pebble
var updateList = function() {
	////console.log("Updating list of LockNames and LockIDs to send to the Pebble");
	//send the number of locks
	console.log("Sending the lock count of " + lockNames.length);
	sendAppMessage({"lockCount": lockNames.length});
	//Pebble.sendAppMessage({"lockCount": lockNames.length, 0: "Hanlon\'s Lockitron",},
  //function(e) {
    //console.log("Successfully delivered message with transactionId=" + e.data.transactionId);
  //},
  //function(e) {
    //console.log("Unable to deliver message with transactionId="+ e.data.transactionId+ " Error is: " + e.error.message);
  //}
//);
	for ( var i = 0, ii = lockNames.length; i < ii; ++i ) {
		//go through each lockName and send it to the Pebble
		//console.log("Sending lockNames[" + i + "] = " + lockNames[i]);
		sendAppMessage({"lockName": lockNames[i]});
	}
};

//function to save the lockList and lockId
var saveState = function() {
	////console.log("Save LockNames of length:" + lockNames.length + " into memory");
	for ( var i = 0, ii = lockNames.length; i < ii; ++i ) {
		localStorage.setItem(i, lockNames[i]);
		localStorage.setItem(i+10, lockIDs[i]);
	}
	//save the lockCounts
	localStorage.setItem("lockCounts", lockNames.length);
	////var tempLockNames = localStorage.getItem(0);
	////console.log("Text loading thefirstlockName:" + tempLockNames + " from memory");
	
};  
function findStringInString(needle, haystack) {
	if (haystack.search(needle + "\"") != -1) {
		return haystack.substr(haystack.search(needle + "\"")+needle.length+1);
	} else {
		return -1;
	}
}
function findStringInQuotes(needle) {
	return needle.match(/"([^"]+)"/)[1];
}
// Request locks using the accessToken
var requestLocks = function() {
	////console.log('requesting locks!');
	//create a url that includes the access token
	var url = lockitronUrl + "?" + accessToken;
	//let the user know that your requesting locks
	Pebble.sendAppMessage({50: "Refreshing locks..."});
	//start a new XMLHttpRequest
	var req = new XMLHttpRequest();
	//open a POST request
	req.open('POST', url, false);
	//what to do once the HttpRequest loads
	req.onload = function() {
		////console.log("onload");
		//if the onload was successful
		if (req.readyState == 4 && req.status == 200) {
			//start a lockCount used in the while statement
			var lockCount = 0;
			//empty the lockNames and lockIDs so that they can be filled
			lockNames = [];
			lockIDs = [];
			//store the responseText into a searchString variable that will be modified in case we want to move back to it
			var searchString = req.responseText;
			//look for the word lock in searchString before the while loop starts
			searchString = findStringInString("lock",searchString);
			//start the while loop
			while (searchString != -1) {
				//modify the search string to start after id
				searchString = findStringInString("id",searchString);
				//store the id value that was in qoutes into an id variable
				var id = findStringInQuotes(searchString);
				//modify the search string to start after name
				searchString = findStringInString("name",searchString);
				//store the name value into a variable
				var name = findStringInQuotes(searchString);
				//fill the lockNames and lockIDs
				lockNames[lockCount] = name;
				lockIDs[lockCount] = id;
				//log what was put into the lockNames and lockIDs
				////console.log("LockName[" + lockCount + "]:" + lockNames[lockCount] + " LockIDs[" + lockCount + "]:" + lockIDs[lockCount]);
				//increment the lockCount
				lockCount++;
				//look for the word lock in searchString
				searchString = findStringInString("lock",searchString);
			} //end of while statement that checks for not -1
			//save the lockNames and lockIDs to memory
			saveState();
			//send the updated list to the Pebble
			updateList();
			//update the lockIndex
			updateLockIndex();
		} else {
			//log the ready state and status for troubleshooting
			////console.log("Ready State:" + req.readState + " and status:" + req.status);
			//let the user know that requesting the locks failed
			Pebble.sendAppMessage({50: "Error. Please Refresh"});
		} //end of else with a readystate other than 4 and a status of 200
	}; //end of onload function
	//send/start the XMLHttpResquest
	req.send(null);
}; //end of requestLocks function

//a variable function to launch the web view
var myfirstwebview = function(){
  Pebble.openURL('https://api.lockitron.com/oauth/authorize?client_id=4f747f5505def6d62fa3f1ff6fbdcb172c5151bed140d74a7c213f348ba69325&response_type=token&redirect_uri=pebblejs://close');
};
//now use that webview function to show the configuration page
Pebble.addEventListener('showConfiguration', myfirstwebview); 

//listen for the closing of the webview and save the access token
Pebble.addEventListener("webviewclosed", function(e) {
	if (e.response === 'CANCELLED') { return; }
	//split the access token out of the response
	accessToken = e.response.split("&")[0];
	//it would be good to check the length of the access token at this point and tell the user they need to relaunch the webview
	////console.log("hey look it's an accessToken: " + accessToken + "zero");
    //store the access token in localStorage
	localStorage.setItem("accessToken", accessToken);
	//now that we have an access token we should request lockNames and lockIDs
	requestLocks();
} //end of function e bracket
); //end of eventViewClosed listener

//load the state of the locks (either from memory or by requestLocks)
var loadState = function() {
	////console.log('Loading State!');
	//var savedLockNames = [];
	//var savedLockIds = [];
	lockCounts = localStorage.getItem("lockCounts");
	//load lockNames and lockIDS from localStorage
	for ( var i = 0, ii = lockCounts; i < ii; ++i ) {
		lockNames[i] = localStorage.getItem(i);
		console.log("locknames["+i+"] is being set to " + lockNames[i]);
		lockIDs[i] = localStorage.getItem(i+10);
		////console.log("savedlockNames["+i+"] is:" +lockNames[i]+ " and savedlockIDs ["+ i+"] is:"+lockIDs[i]);
	}
	//if the saveList isn't null then load it into the current locklist 
	if (lockNames !== null && lockIDs !== null) {
		////console.log("saved names and locks were not null");
		//lockNames = savedLockNames;
		//lockIDs = savedLockIds;
		//why update the lockindex?
		updateLockIndex();
		//send the list of lock names and IDs to the pebble
		////console.log("LockNames Count:" + lockNames.length);
		updateList();
	} else { //otherwise if either are null then request new locks
		////console.log("saved names and locks were null so we're questing locks");
		requestLocks();
	}
};
				
Pebble.addEventListener("ready", function() {
	//lets start the show
	////console.log("5. ready called!");
	//set initialized to true
	initialized = true;
	readyState = true;
	//load access_token from local Storage
	accessToken = localStorage.getItem("accessToken");
	//if accessToken is null then load the webview to get it.
	if (accessToken === null) {
		////console.log("Access log is null in memorry");
		//tell the user on screen that they need to open the PebbleApp
		sendAppMessage({"selectText": "Open Pebble App to config"});
		//open the webview for them if you can
		
	} else { //if the accessToken is set then load the lockNames and lockIDs
		//then load the state (either from memory or by requesting lockNames and lockIDs
		loadState();
	}
});

//listen for messages from the Pebble
Pebble.addEventListener("appmessage", function(e) {
	if (JSON.stringify(e.payload) === "{\"1\":\"unlock\"}") {
		var req = new XMLHttpRequest();
		console.log(JSON.stringify(e.payload));
		req.open('POST', lockitronUrl + "/" + "/unlock?" + accessToken, true);
		req.onload = function(e) {
			console.log("onload");
			if (req.readyState == 4 && req.status == 200) {
				console.log("4 and 200");
				if(req.status == 200) {
					console.log('Unlocked');
				} else {
					console.log("Error");
				}
			} else {
				console.log(req.readyState);
				//console.log(req.status);
			}
		};
	} else if (JSON.stringify(e.payload) === "{\"1\":\"lock\"}") {
		console.log(JSON.stringify(e.payload));
		req.open('POST', 'https://api.lockitron.com/v1/locks/9a3f7cd5-7ba1-44a6-ad8c-16d4b8e5ec15/lock?access_token=42b09d104c2a57832ede8c3e0c0152609ffbb16975cdbc686c1bbf886e5e1386', true);
		req.onload = function(e) {
			console.log("onload");
			if (req.readyState == 4 && req.status == 200) {
				console.log("4 and 200");
				if(req.status == 200) {
					console.log('Locked');
				} else {
					console.log("Error");
				}
			} else {
				console.log(req.readyState);
				// console.log(req.status);
			// }
		// };
	// }
  // req.send(null);
}
);
var maxTriesForSendingAppMessage = 3;
var timeoutForAppMessageRetry = 3000;

function sendAppMessage(message) {
	var numTries = 0;
	if (numTries < maxTriesForSendingAppMessage) {
		numTries++;
		console.log('Sending AppMessage to Pebble: ' + JSON.stringify(message));
		Pebble.sendAppMessage(
			message, function() {}, function(e) {
				console.log('Failed sending AppMessage for transactionId:' + e.data.transactionId + '. Error: ' + e.data.error.message);
				setTimeout(function() {
					sendAppMessage(message);
				}, timeoutForAppMessageRetry);
			}
		);
	} else {
		console.log('Failed sending AppMessage ' + JSON.stringify(message));
	}
}
//noteworthy function for sending messages
//function SendAppMessage (thekey, thecontent) {
	//console.log("Sending key:" + thekey + " with content:" + thecontent);
	//var transactionId = Pebble.sendAppMessage({thekey: thecontent},
		//function(e) {
			//console.log("Successfully delivered message:"+ e.data.message + " with transactionId=" + e.data.transactionId);
		//},
		//function(e) {
			//console.log("Unable to deliver message with transactionId=" + e.data.transactionId + " Error is: " + e.message);
		//}
	//);
//}