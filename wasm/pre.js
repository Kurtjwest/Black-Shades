// Runs before main(): give the game somewhere to keep config.txt and the high
// scores that survives a reload.  IndexedDB is asynchronous, so the run is
// held back until the mount has been read in - otherwise the game would read
// its settings from an empty directory.
Module['preRun'] = Module['preRun'] || [];
Module['preRun'].push(function () {
  try {
    FS.mkdir('/blackshades');
    FS.mount(IDBFS, {}, '/blackshades');
    Module.addRunDependency('blackshades-settings');
    FS.syncfs(true, function (err) {
      if (err) console.warn('could not read saved settings:', err);
      Module.removeRunDependency('blackshades-settings');
    });
  } catch (e) {
    console.warn('settings will not persist:', e);
  }
});

// The browser will not start audio until the page has been clicked, and the
// game wants a click on its menu anyway.
Module['postRun'] = Module['postRun'] || [];
Module['postRun'].push(function () {
  var resume = function () {
    if (typeof Module['SDL2'] !== 'undefined' && Module['SDL2'].audioContext &&
        Module['SDL2'].audioContext.state === 'suspended') {
      Module['SDL2'].audioContext.resume();
    }
  };
  ['click', 'keydown', 'touchend'].forEach(function (ev) {
    window.addEventListener(ev, resume, { once: false });
  });
});
