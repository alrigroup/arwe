// app ARWE - front-end JS (bundled by esbuild)
window.ARWE && ARWE.ready(function (bridge) {
  const el = document.getElementById('app');
  if (el) el.textContent = 'ARWE pronto (' + bridge.version + ')';
});