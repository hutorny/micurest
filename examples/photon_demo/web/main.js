/**
 * main.js - bootstrap JS script for micurest demo applications 
 * HTML page that loads this script is expected to set three variables:
 * CFG, VIEW, and DATA
 * where CFG is an object pointing to htpl elements, 
 * VIEW is a name of html resource, relative to this file
 * 
 * NOTE: do not move main.js to /js/ folder, it will require changes in C/C++
 *  
 * var VIEW = "view.html";    // view path is relative to main.js
 * var CFG = { 
 *   "frame" : "iframe",      // selector of the iframe element, (default is iframe)
 *   "msgbox" : "#msgbox"     // id of the diagnostics message box  
 * };
 * var DATA = { };
 */

/*
 * Bootstraping 
 */
if( ! CFG ) CFG = {};
if( ! CFG.frame ) CFG.frame = "iframe";
if( ! CFG.msgbox ) CFG.frame = "msgbox";
try {
	var scripts = document.getElementsByTagName("script");
	var thisURL = (document.currentScript || scripts[scripts.length - 1]).src;
	var parts = (thisURL && thisURL.split("/")) || [""];
	parts.pop();   // main.js
	CORS = parts.join("/");
	var msgbox = document.querySelector(CFG.msgbox ||"#msgbox");
	if( msgbox ) msgbox.innerText = "Loading " + window.location.toString();
} catch(e) { console.log(e); };

/*
 * Main bootsrapper class
 */
class UIMain {
	one(sel) {
		return (typeof sel === 'string') ? document.querySelector(sel) : sel;
	}
	
	init(data) {
		this.model = data;
		this.frame = this.one(CFG.frame);
		this.msgbox = this.one(CFG.msgbox) || document.createElement("span");
		this.msgtext = this.msgbox.childNodes[0]; 
		this.view = CORS + "/" +VIEW;
		this.msgbox.innerText = "Loading " + CORS + '/' + VIEW;
		if( this.frame )
			this.loadFrame();
		else
			console.log("No element matching : '" + CFG.frame);
	}
	
	getText(url, fn, to) {
		var xhr = new XMLHttpRequest();
		xhr.onreadystatechange = function () {
			if( this.readyState == 4 ) {
				if( this.status == 200 ) //TODO handle other good response codes
		        	fn(null,this.responseText);
			}
		};
		xhr.open('GET', url);
		if( to ) {
			xhr.timeout = to;
			xhr.ontimeout = function () { 
				fn({message : url + ' is not accessible'}, null); };
		}
		xhr.send(null);
	}
	
	loadFrame() {
		var origin = location.origin;
		var ui = this;
		this.frame.contentWindow.ORIGIN = origin;
		this.frame.contentWindow.MSGBOX = this.msgbox;
		this.frame.onerror = function() {
			this.msgbox.innerText = 'Unable to load ' + this.view;			
		}
		this.frame.onload = function() { 
			this.contentWindow.ORIGIN = origin;
			this.contentWindow.MSGBOX = this.msgbox;
			if( typeof(this.contentWindow.render) === 'function' )
				this.contentWindow.render(ui.model);
//			else
//				console.log('no render function provided');
			ui.msgbox.style.display = 'none';
			ui.msgtext.innerText = 'OK';
			this.style.display = 'block';
			var doc = this.contentDocument || this.contentWindow.document;
			document.title = doc.title;
			var link = ui.frame.contentDocument.querySelector('link[rel="shortcut icon"]');
			if( link ) document.head.appendChild(link.cloneNode());
		}
		this.getText(this.view, function(err, data){
			if( data != null ) {
				var doc = ui.frame.contentDocument || ui.frame.contentWindow.document;
				if( doc == null ) {
					console.log('No target document');
					return;
				}
				doc.open();
				doc.write(data);
				doc.close();
			}
		}, 10000);
	};
}

var UI = new UIMain();
setTimeout(function(){ UI.init(DATA); },0);
