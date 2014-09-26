var initialized = false;
//lockitron variables
var lockitronUrl = 'https://api.lockitron.com/v1/locks/';
var lockNames = [];
var lockIDs = [];
var lockCounts = 0;
var readyState = false;
//load access_token from local Storage
var accessToken = localStorage.getItem("accessToken");				

//update the list of locks and send it to the Pebble
var updateList = function() {
	////console.log("Updating list of LockNames and LockIDs to send to the Pebble");
	//send the number of locks
	//console.log("Sending the lock count of " + lockNames.length);
	sendAppMessage({"lockCount": lockNames.length});
	
	//go through each lockName and send it to the Pebble
	for ( var i = 0, ii = lockNames.length; i < ii; ++i ) {
		//console.log("Sending lockNames[" + i + "] = " + lockNames[i]);
		sendAppMessage({"lockName": lockNames[i]});
	}
};

//function to save the lockNames and lockId
var saveState = function() {
	////console.log("Save LockNames of length:" + lockNames.length + " into memory");
	for ( var i = 0, ii = lockNames.length; i < ii; ++i ) {
		//save lockNames into the first 10 index's
		localStorage.setItem(i, lockNames[i]);
		//save lockIDs into the next 10
		localStorage.setItem(i+10, lockIDs[i]);
	}
	//save the lockCounts
	localStorage.setItem("lockCounts", lockNames.length);
};  

//find String In String function used to extrack lock names and IDs from json locks response
function findStringInString(needle, haystack) {
	if (haystack.search(needle + "\"") != -1) {
		return haystack.substr(haystack.search(needle + "\"")+needle.length+1);
	} else {
		return -1;
	}
}

//find String In Quotes function used to extrack lock names and IDs from json locks response
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
	
	//set initialized to true
	initialized = true;
	readyState = true;
	//load access_token from local Storage
	accessToken = localStorage.getItem("accessToken");
	console.log("access token:" + accessToken);
	//if accessToken is null then load the webview to get it.
	if (accessToken === null) {
		//tell the user on screen that they need to open the PebbleApp
		sendAppMessage({"selectText": "accessToken is null"});
		//open the webview for them if you can
		
	} else { //if the accessToken is set then load the lockNames and lockIDs
		//then load the state (either from memory or by requesting lockNames and lockIDs
		loadState();
	}
});

//listen for messages from the Pebble
Pebble.addEventListener("appmessage", function(e) {
	//stringify the payload
	var message = JSON.stringify(e.payload);
	//iterate through the locks
	for ( var i = 0, ii = lockNames.length; i < ii; ++i ) {
		console.log("message: " + message);
		//check to see if the lock name mataches the message
		if (message.indexOf(lockNames[i]) != -1) {
			console.log("lock being requested is " + lockNames[i]);
			sendAppMessage({"statusText": "Knock on door!"});
			if (message.indexOf("lock") == 2) {
				console.log("locking " + lockNames[i]);
				sendHttpRequest(lockIDs[i], "lock");
			} else if (message.indexOf("unlock") == 2) {
				console.log("unlocking " + lockNames[i]);
				sendHttpRequest(lockIDs[i], "unlock");
			} else {
				console.log("key was invalid");
			}
		}
	}
});
function sendHttpRequest (lockID, action) {
	var req = new XMLHttpRequest();
	req.open('POST', lockitronUrl + lockID + "/" + action + "?" + accessToken, true);
	req.onload = function(e) {
		if (req.readyState == 4 && (req.status == 200 || req.status == 500)) {
			//console.log("Success");
			sendAppMessage({"statusText": action + "ed!"});
		} else {
			console.log("Error. readyState:" + req.readyState + " status:" + req.status);
		}
	};
	req.send(null);
}

//sendAppMessage taken from https://github.com/Neal/pebble-vlc-remote and modified based on the use case
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