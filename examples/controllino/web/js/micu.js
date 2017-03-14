/**
 * Copyright (C) 2017 Eugene Hutorny <eugene@hutorny.in.ua>
 *
 * micu.js - UI controller
 *
 * This file is part of µcuREST Library. http://hutorny.in.ua/projects/micurest
 *
 * The µcuREST Library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License v2
 * as published by the Free Software Foundation;
 *
 * The µcuREST Library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License v2
 * along with the COJSON Library; if not, see
 * <http://www.gnu.org/licenses/gpl-2.0.html>.
 */

/**
 * Sugar for query selectors 
 */
function $all(sel) {
	return (typeof sel === 'string') ? document.querySelectorAll(sel) : sel;
}

function $arr(sel) {
	if( sel === null ) return [];	
	return (typeof sel === 'string') 
		? Array.apply(null, $all(sel))
		: Array.apply(null, sel);
};

function $one(sel) {
	return (typeof sel === 'string') ? document.querySelector(sel) : sel;
};

function $resname(url) {
	if( ! url ) return url;
	var p = url.split('/');
	if( p[1] === '' ) {
		p.shift();	// http
	}
	if( p[0] === '' ) {
		p.shift();	// ''
		p.shift();	// host
	}
	return p.join('/');
}

function $for(sel, fn, ths) {
	if(sel) $arr(sel).forEach(fn, ths);
}

function isCheckable(element) {
	return element ? (element.type === 'radio' || element.type === 'checkbox') : false;
}

$for.escape = function(s) {
	return s.replace($for.escape.regexp, "\\$1");	
};

$for.escape.regexp = /([!"#$%&'()*+,.\/:;<=>?@\[\\\]^`{|}~])/g; 

$one.escape = $for.escape;

class Timeout {
	constructor(message) {
		this.message = message;
	}
}
/* originated from 
 * http://stackoverflow.com/questions/201183/how-to-determine-equality-for-two-javascript-objects
 */
function equals(x, y) {
	return (x && y && typeof x === 'object' && typeof y === 'object') ?
		(Object.keys(x).length === Object.keys(y).length) &&
			Object.keys(x).every(function(key) {
				return equals(x[key], y[key]);
			}, true) : (x === y);
}

/**
 *  Utility functions
 */
function isTrueNaN(val) { return typeof val === 'number' && isNaN(val); }

class UICommon {
	constructor() {
		this.model = null;
		this.directives = {};
		this.targets = {};
		this.handlers = {};
		this.controls = {};
		this.timeout = 1000;
	}
	showMessage(/*...*/) {		
		//this.showPopup('info', joinvarargs(arguments,'\n'));
	};
	
	errorMessage(/*...*/) {
		//this.showPopup('error', joinvarargs(arguments,'\n'));
	};
	
	commError(err) {
		console.log(err);
		//TODO shows errors to the user
	};	

	
//	showPopup(id, message) {
//		$one('#'+id+'-message-text').innerHTML = message;
//		$one('#'+id+'-show').checked = true;		
//	};
	
	rebase() {
	 	$for($all('a[href^="/"]'), function(a) {
	 		a.href = ORIGIN + '/' + $resname(a.href);
	 	});		
	 	$for($all('img[src^="/"]'), function(img) {
	 		img.src = ORIGIN + '/' + $resname(img.src);
	 	});		
	}
	convertTo(type, value, id) {
		switch(type) {
		case typeof(0): return Number(value);
		case typeof(false): return Boolean(value);
		case typeof({}): return this.convertToObject(value, id);
		}
		return value;
	}
	
	convertToObject(value, id) {
		try { return JSON.parse(value); } catch(e) {}
		return value;
	}

	send(method, url, text, contentType, fn) {
		var xhr = new XMLHttpRequest();    	
		xhr.onreadystatechange = function () {
			if( this.readyState == 4 ) {
				if( this.status >= 200  && this.status <= 204 )
		        	fn(null, this.responseText, this.getResponseHeader('Content-Type'), this.statusText);
		        else {		        	
			       	fn(new Error(url), this.responseText, 'error/text', this.statusText);
		        }
			}
		};
	    xhr.open(method, url);
	    if( this.timeout ) {
	    	xhr.timeout = this.timeout;
	    	xhr.ontimeout = function () { fn(new Timeout('Controller did not respond in timely fasion'), null); };
	    }	
	    if( contentType !== null ) {
	    	xhr.setRequestHeader(method === 'GET'?'Accept':'Content-Type', contentType);
	    }
		xhr.send(text);
		return xhr;
	};
	
	get(url, contentType, fn) {
		return this.send('GET', url, null, contentType, fn);
	}	
	getText(url, fn) {
		return this.get(url, 'text/plain', fn); 
	};
	getJSON(url, fn) {
		return this.get(url, 'application/json', function(err, obj) {
			if( obj ) try {
				obj = JSON.parse(obj);
			} catch(e) { err = e; obj = null; }
			fn(err, obj);
		});
	};
	putText(url, text, contentType, fn) {
		if( ! text ) text = '\r\n';
		else if( text.slice(-2) !== '\r\n' ) text = text + '\r\n';  
		return this.send('PUT', url, text, contentType ?contentType:'text/plain', fn);
	};
	putJSON(url, obj, fn) {
		var text = JSON.stringify(obj);
		if( ! text ) text = '\r\n'; //FIXME is this needed yet?
		else if( text.slice(-2) !== '\r\n' ) text = text + '\r\n';  
		return this.send('POST', url, text, 'application/json', function(err, data, type, status) {
			if( err ) return fn(err, data, type, status);
			var json_data;
			if( type === 'application/json' ) try {
				json_data = JSON.parse(data);
				return fn(err, json_data, type, status);				
			} catch(e) { console.log(e); }
			return fn(err, data, type, status);
		});
	};
	putField(field) {
		var ui = this;
		var value = isCheckable(field) ? field.checked : this.convertTo(field.dataType, field.value, field.id);
		var url = '/' + (this.directives.bind[field.id] || field.id);
		this.putText(ORIGIN + url, 
			(field.contentType === 'application/json' || value === null ) 
				? JSON.stringify(value)	:  value.toString(), 
			 field.contentType ? field.contentType : 'plain/text', function(e,d,c,s) { ui.putcb(e,d,c,s); });					
	}
	putcb(err, data) {
		if( err ) console.log(err);
	}	
	collect(directives, doc, onchange, def) {
		var obj = def || {};
		if( typeof doc === 'undefined' ) doc = document;
		var keys = Object.keys(directives);
		for(var i in keys) {
			var k = keys[i];
			if( k[0] == '{' ) {
				obj[k.slice(1)] = this.collect(directives[k], doc, onchange);
			} else {
				if( k[0] == '[' ) {
					obj[k.slice(1)] = this.collect(directives[k], doc, onchange, []);
				} else {
					var p = k.split('@');
					if( ! p[1] ) p[1] == 'value';
					var e = doc.querySelector(p[0]);
					if( e ) {
						var val = e.className.includes('type-int') 
							? parseInt(e[p[1]])
							: e.className.includes('type-float')
							? parseFloat(e[p[1]]) 
							: e[p[1]];
						if( ! isTrueNaN(val) )
							obj[directives[k]] = val;
						if( onchange )
							e.onchange = onchange;
					}
				}
			}
		}
		return obj;
	}
	fixAbout() {
		if( ORIGIN && this.controls.about ) {
			var about = $one(this.controls.about);
			var origin = ORIGIN.split('/')[2]; 
			if( about ) {
				about.setAttribute('href',about.getAttribute('href') + '#' + origin);
			}
		}		
	}
}

class UIStatus extends UICommon {
	constructor() {	
		super();
	}
	init() {
		var ui = this;	
		var mcu_restart = $one('#mcu_restart');
		if( mcu_restart ) mcu_restart.onclick = function() {
			ui.putText(ORIGIN + '/restart', this.id, "text/plain",function(){}); 
		}
		this.hook(this.isautoupdate());
	}
	
	hook(auto) {
		var ui = this;	
		this.autoupdate = $one('#autoupdate');
		
		if( this.autoupdate ) {
			this.autoupdate.checked = auto;
			this.autoupdate.onchange = function() {
				if( this.checked ) ui.updatertdata();
			}			
		}		
	}
	
	isautoupdate() {
		return this.autoupdate && this.autoupdate.checked;
	}
	
	render(obj) { try {
		this.model = obj;
	 	this.rebase();
	 	this.fixAbout();
	 	$p('#sysinfo').render(obj, this.directives.sysinfo);
	 	$p('#interfaces').render(obj, this.directives.interfaces);
	 	this.hook();
	} catch(e) { console.log(e); }}
	
	renderrt(obj) { try {
		var auto = this.isautoupdate();
	 	$p('#sysinfo').render(obj, this.directives.rtdata);
	 	this.hook(auto);
	} catch(e) { console.log(e); }}
	
	updatertdata() {
		var ui = this;
		if( ui.rttimer ) clearTimeout(ui.rttimer);
		ui.rttimer = null;
		this.getJSON(ORIGIN + '/sysinfo/rtdata', 
			function(err, obj) {
				if( obj ) ui.renderrt(obj);
				else console.log(err);
				if( ui.isautoupdate() )
					ui.rttimer = setTimeout(function() { ui.updatertdata() }, 1000);
		})
	}
}

class UISetup extends UICommon {
	constructor() {	
		super();
		var ui = this;
		this.handlers = {
			apply : function() { ui.apply(); },
			applied : function(err, data) { ui.applied(err, data); },
			save : function() { ui.save(); }
		}
	}
	
	render(obj) { try {
		this.model = obj;
		this.rebase();
	 	this.fixAbout();
	 	$p('#fieldsets').render(obj, this.directives.config);		
	} catch(e) { console.log(e); }}

	apply() {
		var obj = this.collect(this.directives.collect);
		try { obj.ap.dhcps.range.enable = (!!obj.ap.dhcps.range.start && !!obj.ap.dhcps.range.end); } catch(e) {}
		this.putText(ORIGIN + '/config/current', JSON.stringify(obj) + '\r\n', 
			'application/json', this.handlers.applied );
	}

	save() {
		var obj = this.collect(this.directives.collect);
		try { obj.ap.dhcps.range.enable = (!!obj.ap.dhcps.range.start && !!obj.ap.dhcps.range.end); } catch(e) {}
		this.putText(ORIGIN + '/config/default', JSON.stringify(obj) + '\r\n', 
			'application/json', this.handlers.applied );
	}
	
	applied(err, data) {
		if( err ) console.log(err);
	}
	
	init() {
		var ui = this;
		$one('#actions #apply').onclick = this.handlers.apply;
		$one('#actions #save').onclick = this.handlers.save;
	}
}

class UIDemo extends UICommon {
	constructor() {	
		super();
	}
	upload(url, file, fn){
		var ui = this;
		var reader = new FileReader();
		reader.onload = function(e) {
			ui.send('PUT', url, e.target.result, 'application/octet-stream', fn);			
		}
		reader.readAsArrayBuffer(file);
	}
	uploaded() {
		if( this.img ) {
			var src = this.img.getAttribute('src');
			this.img.removeAttribute('src');
			var img = this.img;
			setTimeout(function() { img.setAttribute('src',src); }, 0);
		}
	}
	init() { 
		var ui = this;
		this.log = $one('#log');
		this.img = $one(UI.targets.blob);
		$for('.bind',function(b) {
			if( isCheckable(b) ) {
				$for('[name="'+b.name+'"]', function(g) {
					g.onchange = function(e) {
						ui.putField(b);
					};
				});
			} else {
				b.onkeypress = function(e) {
					if( e.code == 'Enter' ) {
						ui.putField(this);
					}
				}
		}
		});
	}
	convertToObject(value, id) {		
		console.log('Convering "' + value + '" + for ' + id);
		if( id === 'rtc' )
		try { 
			var d = new Date(value);
			return {
			    year:   d.getFullYear()-2000,
			    month:  d.getMonth() + 1,
			    day:    d.getDate(),
			    weekday:d.getDay(),
			    hour: 	d.getHours(),
			    minute:	d.getMinutes(),
			    second:	d.getSeconds()
			};   
		} catch(e) {}
		return value;
	}
	
	
	putcb(err, data, type, status) {
		if( err ) console.log(err);
		if( this.log ) {
			if( err )
				this.log.innerHTML += '<span style="color:red">' + [err, JSON.stringify(data)].join(' ') + '</span><br>'; 
			else
				this.log.innerHTML += '<span>' + [status, type, data].join(' ') + '</span><br>';
		}
	}	

	modes(list) {
		$p(this.targets.mode).render(list, this.directives.mode);
	}
	
	editable(obj) {
		if(obj.hasOwnProperty('year')) {
			var d = new Date(obj.year+(obj.year<100?2000:0), obj.month-1, obj.day, obj.hour, obj.minute, obj.second);
			d = new Date(d.valueOf() - (d.getTimezoneOffset()*60*1000));
			return d.toISOString().replace(/\.[0-9]+Z/,'').replace('T', ' ');
		} else
			return JSON.stringify(obj);
	}
	setrtc() {
		var ui = this;
		var obj = this.convertToObject(new Date(),'rtc');
		this.putJSON(ORIGIN + '/rtc', obj, function(e,data,c,s) {			
			if( data && ui.rtc ) try {
				var date = JSON.parse(data);
				ui.rtc.value = ui.editable(date); 
			} catch(e) {}
			ui.putcb(e,data,c,s); 
		});
	}
	render(obj) { 
		this.rebase();
		var ui = this;
		if( obj ) ui.modes(obj);
	 	this.fixAbout();		
		if( this.targets.rtc ) 
			this.rtc = $one(this.targets.rtc);			
		if( this.targets.setrtc ) {
			var setrtc = $one(this.targets.setrtc)
			if( setrtc ) setrtc.onclick = function() {
				ui.setrtc();
			}
		}			
		setTimeout(function() {
			if( obj ) ui.queue($arr('.bind'));
			else
				if( ui.directives.meta && ui.directives.meta.mode ) {
					ui.getJSON(ORIGIN+ui.directives.meta.mode, 
							function(err, list, type) {
								if( list ) ui.modes(list);
								ui.queue($arr('.bind'));
					});
				} else
					ui.queue($arr('.bind'));
		}, 100);
	}
	queue(bind) {		
		if( ! bind  || ! bind.length ) return;
		var ui = this;
		var e = bind.shift();
		var url = '/' + (this.directives.bind[e.id]||e.id);		
		if( e.type === 'file' ) {
			if( !(e.disabled = ! window.FileReader) ) {}
			e.onchange = function() {
				if( this.files.length >0 ) {
					var f = this.files[0]; 
					if( f.size > 1200 ) 
						console.log('Size of file "' + f.name +'" is ' + f.size +' bytes and exceed upload limits of 1200 bytes');
					ui.upload(ORIGIN+url, this.files[0], 
						function(err, data, type, status){
							ui.putcb(err,data, type, status);
							ui.uploaded(); 
					});
				}
			};
			return ui.queue(bind); // recursion for simplicity
		}
		this.getText(ORIGIN + url, function(err, data, type) {
			if( err ) console.log(err);
			else {
				if( type === 'application/json' )
					try { data = JSON.parse(data) } catch(e) { console.log(e); } 
				if( isCheckable(e) ) {
					if( e.name )
						$for('input[name="' + e.name + '"]', function(i) {
							i.disabled = false;
						});
					e.checked = data;
				} else {
					if( typeof(data) === typeof({}) )
						e.value = ui.editable(data);
					else
						e.value = data;
				}
				e.disabled = false;
				e.contentType = type;
				e.dataType = typeof(data);
				if( e.type == 'select-one' ) {
					e.onchange = function() { 
						ui.putField(this);
					}
				}
			}
			ui.queue(bind);
		});
	}
}

class UIGpio extends UICommon {
	constructor() {	
		super();
		this.queue = [];
		var ui = this;
		this.handlers = {
		 	port_config : function(data) {
	 			ui.data.port[0].config = data;
		 	},
		 	port : function(data) {
		 		ui.render_port(data);
		 	},
		 	gpios : function(data) {
		 		for(var i in data) {
		 			data[i].url = ui.model.index.gpio[data[i].gpio].url;
		 		}
	 			ui.render_gpios(data);
	 		},
		 	gpios_config : function(data) {
		 		for(var i in data) {
		 			data[i].url = ui.model.index.gpio[data[i].gpio].config;
		 		}
	 			ui.render_gpios_config(data);
		 	},
		 	gpio : function(data) {
	 			ui.render_gpio(data);
	 		},
		 	gpio_config : function(data) {
	 			ui.render_gpio_config(data);
		 	},
		 	mode : function(data) {
	 			ui.render_mode(data);
	 		},
	 		portmode : function() {
	 			if( ! this.getAttribute('url') || ! this.source ) return;
	 			var bits = $arr(this.source);
	 			var mask = 0;
	 			for(var i in bits) {
	 				var bit = bits[i];
	 				if( bit.checked ) { mask |= (1<<(0+bit.getAttribute('pin'))); console.log(mask, 0+bit.getAttribute('pin')); }
	 			}
	 			ui.putJSON(ORIGIN + this.getAttribute('url'), {enable:mask}, ui.handlers.portmodecb);
	 		},
	 		portmodecb : function(err, data) {
	 			if( err ) console.log(err);
	 			else ui.portmodecb(data);
	 		},
	 		portstate : function() {
	 			if( ! this.getAttribute('url') || ! this.source ) return;
	 			var bits = $arr(this.source);
	 			var mask = 0;
	 			for(var i in bits) {
	 				var bit = bits[i];	 				
	 				if( bit.checked ) { mask |= (1<<(0+bit.getAttribute('pin'))); console.log(mask, 0+bit.getAttribute('pin')); }
	 			}
	 			ui.putJSON(ORIGIN + this.getAttribute('url'), mask, ui.handlers.portstatecb);
	 		},
	 		portstatecb : function(err, data) {
	 			if( err ) console.log(err);
	 			else ui.portstatecb(data);
	 		}
		};
		this.controls = {
			gpio : { 
				handler : function() {
					var control = this;
					ui.putJSON(ORIGIN + this.getAttribute('url'), this.checked + 0,
						function(err, data) {
			 				if( err ) console.log(err);
			 				else control.checked = !!data; 
					});
				}	
			},
			gpio_config : {
				handler : function() {
					var control = this;
					console.log(this.value);
					ui.putJSON(ORIGIN + this.getAttribute('url'), { mode : this.value },
							function(err, data) {
				 				if( err ) console.log(err);
				 				else { 
				 					if( typeof(data) === 'object' ) 
				 						control.value = data.mode;
				 					$one('#'+control.id+' input').disabled = data.mode != 'out';
				 				}
						});
				}
			},
			portmode : {},
			portstate : {}			
		};
	}
	init() {
		
	}
	
	merge(dst, src, i) {
		Object.assign(dst ? dst : {}, src, {'id':i});
		Object.keys(dst).forEach(function(p){
			if( typeof(dst[p]) === 'string' ) {
				dst[p] = dst[p].replace('*', i);
			}
		});
		return this.expand(dst);
	}
	
	expand(obj) {
		var ui = this;
		var keys = obj['.'];
		if( ! keys ) return obj;
		var template = obj['*'];
		var arr = ! isNaN(keys[0]); 
		keys.forEach(function(k,i) {
			if( ! obj.hasOwnProperty(k) ) obj[k] = {};
			if( arr ) {
				keys[i] = ui.merge(obj[k], template, k);			
				delete obj[k];
			} else {
				obj[k] = ui.merge(obj[k], template, k);
			}
		});
		delete obj['.'];
		delete obj['*'];
		return arr ? keys : obj;
	}
	
	unmask(bits, mask) {
		var arr = [];
		for(var k in bits) {
			arr[k] = (mask & (1<<bits[k].id)) ? 1 : 0; 
		}
		return arr;
	}

	make_mode_renderer(mode) {
		return function() { 
			return this.mode == mode; 
		};
	}
	
	render_mode(data) {
		var directives = this.directives.gpios_config['.gpio']['p<-'];
		for(var i in data) {
			directives['select option[value="' + data[i] + '"]@selected'] = this.make_mode_renderer(data[i]); 
		}
		try { $p(this.targets.mode).render(data, this.directives.mode); } catch(e) { console.log(e); }		
	}
	
	render_gpios_config(data) {
		data.reverse();
		try { $p(this.targets.gpios_config).render(data, this.directives.gpios_config);} catch(e) { console.log(e); } 
	}
	
	render_gpios(data) { 
		data.reverse();
		for(var i in data) {
			data[i]['in'] = !! data[i]['in']; 
		}
		try { $p('#gpio-bits').render(data, this.directives.gpios);} 
		catch(e) { console.log(e); }
	}
	
	index(model) {
		var res = {gpio:{}};
		for(var i in model.gpio) {
			res.gpio[model.gpio[i].id] = model.gpio[i]; 
		}
		return res;
	}
	
	render_port() {
		var port = this.data.port[0];
		port.pin  = this.unmask(port.meta.pin, port.data);
		port.enable = this.unmask(port.meta.pin, port.config.enable);
		if( ! this.directives.state ) {
			var state = {};
			var mode = {}
			var ui = this;
			for(var k in port.meta.pin) {
				state['#bit-state-' + port.meta.pin[k].id + ' input@checked'] = function(x) {
					return function() {  return !! ui.data.port[0].pin[x]; };
				}(k);
				state['#bit-state-' + port.meta.pin[k].id + ' input@disabled'] = function(x) {
					return function() {  return ! ui.data.port[0].enable[x]; };
				}(k);
				mode['#bit-mode-' + port.meta.pin[k].id + ' input@checked'] = function(x) {
					return function() {  return !! ui.data.port[0].enable[x]; };
				}(k);
			}
			this.directives.state = state;
			this.directives.modes = mode;
		}
		try { $p(this.targets.state).render(port, this.directives.state); } catch(e) { console.log(e); }
		try { $p(this.targets.modes).render(port, this.directives.modes); } catch(e) { console.log(e); }
	}
	
	render(obj) { 
		var ui = this;
		this.rebase();
	 	this.fixAbout();				
	 	this.queue.push({url : ORIGIN + obj.gpio.mode, fn : this.handlers.mode});
	 	this.queue.push({url : ORIGIN + obj.gpio.url, fn : this.handlers.gpios});
	 	this.queue.push({url : ORIGIN + obj.gpio.config, fn : this.handlers.gpios_config});
		this.model = this.expand(obj);
		this.model.index = this.index(this.model);
		this.model.port[0].pin.reverse();
		try { $p(this.targets.port).render(this.model.port[0], this.directives.port); }
		catch(e) { console.log(e); }
	 	this.data = { port : [{meta:this.model.port[0]}]};
	 	this.queue.push({url : ORIGIN + this.model.port[0].config, fn : this.handlers.port_config});
	 	this.queue.push({url : ORIGIN + this.model.port[0].url, fn : this.handlers.port});
	 	this.run();
	}
	run() {
		var ui = this;
		var e = this.queue.shift();
		if( ! e ) {
			this.hookup();
			return;
		}
		this.getJSON(e.url,function(err,data){
			if(err) ui.commError(err);
			else e.fn(data);
			ui.run();
		})
	}
	portmodecb(data) {
		console.log(data);
	}
	portstatecb(data) {
		console.log(data);
	}
	hookup() {		
		var action = $one(this.controls.portstate.selector);
		if( action ) {
			action.source = this.sources.portstate;
			action.onclick = this.handlers.portstate;
		}
		action = $one(this.controls.portmode.selector);
		if( action ) {
			action.source = this.sources.portmode;
			action.onclick = this.handlers.portmode;
		}
		action = $arr(this.controls.gpio.selector)
		for(var i in action ) {
			action[i].onchange = this.controls.gpio.handler;			
		}
		$for(this.controls.gpio_config.selector, function(a) {
			a.onchange = this.controls.gpio_config.handler;			
		}, this);
		
	}
}

class UIIO extends UICommon {
	constructor() {	
		super();
		var ui = this;
		this.handlers = {
			io : function() {
				ui.setIO(this.getAttribute('uri'), this.checked ? 1 : 0, this);
			},
			clearall : function() {	ui.clearall(this.id); },
			refresh : function() { ui.refresh(this.id);	}
		}
	}
	init() {
	}
	clearall(id) {
		var ui = this;
		if(! id || ! this.model[id] ) return;
		var data = new Array(this.model[id].length).fill(0);
		this.putJSON([ORIGIN,id,'*'].join('/'), data, function(err, data, c, s) {			
			if( data ) try {
				var obj = {}; 
				obj[id] = JSON.parse(data);
				console.log(obj);
				ui.renderdata(obj, ui.directives[id], ui.targets[id]);
			} catch(e) {console.log(e);}
			ui.putcb(err, data, c, s);
		});
	}
	refresh(id) {
		var ui = this;
		this.getJSON([ORIGIN,id,'*'].join('/'),function(err, data, c, s) {			
			if( data ) try {
				var obj = {}; 
				obj[id] = data;
				console.log(obj);
				ui.renderdata(obj, ui.directives[id], ui.targets[id]);
				if( id !== 'in' )
					ui.refresh('in');
			} catch(e) {console.log(e);}
			ui.putcb(err, data, c, s);
		});
	}
	setcb(control, err, data, contype, status) {
		if( err ) console.log(err);
		if( typeof(data) === 'number' ) control.checked = !!data; 
	}
	setIO(uri, val, control) {
		var ui = this;
		this.putJSON(ORIGIN + uri, val,  
			function(err, data, contype, status) { 
				ui.setcb(control, err, data, contype, status); 
			});					
	}
	renderdata(data, dirs, target) {
		var ui = this;
		console.log(data,dirs);
		try { $p(target).render(data, dirs); } 
		catch (e) { console.log(e);	}		
		$for(this.controls.io, function(i) { i.onchange = ui.handlers.io; });
		$for(this.controls.clearall, function(i) { i.onclick = ui.handlers.clearall; });
		$for(this.controls.refresh, function(i) { i.onclick = ui.handlers.refresh; });
	}
	render(obj) { 
		this.model = obj;
		this.rebase();
	 	this.fixAbout();				
		this.renderdata(this.model, this.directives.io, this.targets.io);		
	}
}