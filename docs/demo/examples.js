
  var Module = typeof Module !== 'undefined' ? Module : {};

  if (!Module.expectedDataFileDownloads) {
    Module.expectedDataFileDownloads = 0;
  }

  Module.expectedDataFileDownloads++;
  (function() {
    // When running as a pthread, FS operations are proxied to the main thread, so we don't need to
    // fetch the .data bundle on the worker
    if (Module['ENVIRONMENT_IS_PTHREAD']) return;
    var loadPackage = function(metadata) {

      var PACKAGE_PATH = '';
      if (typeof window === 'object') {
        PACKAGE_PATH = window['encodeURIComponent'](window.location.pathname.toString().substring(0, window.location.pathname.toString().lastIndexOf('/')) + '/');
      } else if (typeof process === 'undefined' && typeof location !== 'undefined') {
        // web worker
        PACKAGE_PATH = encodeURIComponent(location.pathname.toString().substring(0, location.pathname.toString().lastIndexOf('/')) + '/');
      }
      var PACKAGE_NAME = 'docs/demo/examples.data';
      var REMOTE_PACKAGE_BASE = 'examples.data';
      if (typeof Module['locateFilePackage'] === 'function' && !Module['locateFile']) {
        Module['locateFile'] = Module['locateFilePackage'];
        err('warning: you defined Module.locateFilePackage, that has been renamed to Module.locateFile (using your locateFilePackage for now)');
      }
      var REMOTE_PACKAGE_NAME = Module['locateFile'] ? Module['locateFile'](REMOTE_PACKAGE_BASE, '') : REMOTE_PACKAGE_BASE;
var REMOTE_PACKAGE_SIZE = metadata['remote_package_size'];

      function fetchRemotePackage(packageName, packageSize, callback, errback) {
        
        var xhr = new XMLHttpRequest();
        xhr.open('GET', packageName, true);
        xhr.responseType = 'arraybuffer';
        xhr.onprogress = function(event) {
          var url = packageName;
          var size = packageSize;
          if (event.total) size = event.total;
          if (event.loaded) {
            if (!xhr.addedTotal) {
              xhr.addedTotal = true;
              if (!Module.dataFileDownloads) Module.dataFileDownloads = {};
              Module.dataFileDownloads[url] = {
                loaded: event.loaded,
                total: size
              };
            } else {
              Module.dataFileDownloads[url].loaded = event.loaded;
            }
            var total = 0;
            var loaded = 0;
            var num = 0;
            for (var download in Module.dataFileDownloads) {
            var data = Module.dataFileDownloads[download];
              total += data.total;
              loaded += data.loaded;
              num++;
            }
            total = Math.ceil(total * Module.expectedDataFileDownloads/num);
            if (Module['setStatus']) Module['setStatus']('Downloading data... (' + loaded + '/' + total + ')');
          } else if (!Module.dataFileDownloads) {
            if (Module['setStatus']) Module['setStatus']('Downloading data...');
          }
        };
        xhr.onerror = function(event) {
          throw new Error("NetworkError for: " + packageName);
        }
        xhr.onload = function(event) {
          if (xhr.status == 200 || xhr.status == 304 || xhr.status == 206 || (xhr.status == 0 && xhr.response)) { // file URLs can return 0
            var packageData = xhr.response;
            callback(packageData);
          } else {
            throw new Error(xhr.statusText + " : " + xhr.responseURL);
          }
        };
        xhr.send(null);
      };

      function handleError(error) {
        console.error('package error:', error);
      };

      var fetchedCallback = null;
      var fetched = Module['getPreloadedPackage'] ? Module['getPreloadedPackage'](REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE) : null;

      if (!fetched) fetchRemotePackage(REMOTE_PACKAGE_NAME, REMOTE_PACKAGE_SIZE, function(data) {
        if (fetchedCallback) {
          fetchedCallback(data);
          fetchedCallback = null;
        } else {
          fetched = data;
        }
      }, handleError);

    function runWithFS() {

      function assert(check, msg) {
        if (!check) throw msg + new Error().stack;
      }
Module['FS_createPath']("/", "examples", true, true);
Module['FS_createPath']("/examples", "gb_spu", true, true);
Module['FS_createPath']("/examples/gb_spu", "metron", true, true);
Module['FS_createPath']("/examples", "ibex", true, true);
Module['FS_createPath']("/examples/ibex", "metron", true, true);
Module['FS_createPath']("/examples", "j1", true, true);
Module['FS_createPath']("/examples/j1", "metron", true, true);
Module['FS_createPath']("/examples", "pong", true, true);
Module['FS_createPath']("/examples/pong", "metron", true, true);
Module['FS_createPath']("/examples/pong", "reference", true, true);
Module['FS_createPath']("/examples", "rvsimple", true, true);
Module['FS_createPath']("/examples/rvsimple", "metron", true, true);
Module['FS_createPath']("/examples", "tutorial", true, true);
Module['FS_createPath']("/examples", "uart", true, true);
Module['FS_createPath']("/examples/uart", "metron", true, true);
Module['FS_createPath']("/", "tests", true, true);
Module['FS_createPath']("/tests", "metron_bad", true, true);
Module['FS_createPath']("/tests", "metron_good", true, true);

      /** @constructor */
      function DataRequest(start, end, audio) {
        this.start = start;
        this.end = end;
        this.audio = audio;
      }
      DataRequest.prototype = {
        requests: {},
        open: function(mode, name) {
          this.name = name;
          this.requests[name] = this;
          Module['addRunDependency']('fp ' + this.name);
        },
        send: function() {},
        onload: function() {
          var byteArray = this.byteArray.subarray(this.start, this.end);
          this.finish(byteArray);
        },
        finish: function(byteArray) {
          var that = this;
          // canOwn this data in the filesystem, it is a slide into the heap that will never change
          Module['FS_createDataFile'](this.name, null, byteArray, true, true, true);
          Module['removeRunDependency']('fp ' + that.name);
          this.requests[this.name] = null;
        }
      };

      var files = metadata['files'];
      for (var i = 0; i < files.length; ++i) {
        new DataRequest(files[i]['start'], files[i]['end'], files[i]['audio'] || 0).open('GET', files[i]['filename']);
      }

      function processPackageData(arrayBuffer) {
        assert(arrayBuffer, 'Loading data file failed.');
        assert(arrayBuffer.constructor.name === ArrayBuffer.name, 'bad input to processPackageData');
        var byteArray = new Uint8Array(arrayBuffer);
        var curr;
        // Reuse the bytearray from the XHR as the source for file reads.
          DataRequest.prototype.byteArray = byteArray;
          var files = metadata['files'];
          for (var i = 0; i < files.length; ++i) {
            DataRequest.prototype.requests[files[i].filename].onload();
          }          Module['removeRunDependency']('datafile_docs/demo/examples.data');

      };
      Module['addRunDependency']('datafile_docs/demo/examples.data');

      if (!Module.preloadResults) Module.preloadResults = {};

      Module.preloadResults[PACKAGE_NAME] = {fromCache: false};
      if (fetched) {
        processPackageData(fetched);
        fetched = null;
      } else {
        fetchedCallback = processPackageData;
      }

    }
    if (Module['calledRun']) {
      runWithFS();
    } else {
      if (!Module['preRun']) Module['preRun'] = [];
      Module["preRun"].push(runWithFS); // FS is not initialized yet, wait for it
    }

    }
    loadPackage({"files": [{"filename": "/examples/gb_spu/gb_spu_main.cpp", "start": 0, "end": 4066}, {"filename": "/examples/gb_spu/metron/MetroBoySPU2.h", "start": 4066, "end": 26876}, {"filename": "/examples/ibex/README.md", "start": 26876, "end": 26927}, {"filename": "/examples/ibex/main.cpp", "start": 26927, "end": 27068}, {"filename": "/examples/ibex/metron/ibex_alu.h", "start": 27068, "end": 28653}, {"filename": "/examples/ibex/metron/ibex_compressed_decoder.h", "start": 28653, "end": 40751}, {"filename": "/examples/ibex/metron/ibex_multdiv_slow.h", "start": 40751, "end": 55161}, {"filename": "/examples/ibex/metron/ibex_pkg.h", "start": 55161, "end": 71185}, {"filename": "/examples/ibex/metron/prim_arbiter_fixed.h", "start": 71185, "end": 73686}, {"filename": "/examples/j1/main.cpp", "start": 73686, "end": 73805}, {"filename": "/examples/j1/metron/dpram.h", "start": 73805, "end": 74243}, {"filename": "/examples/j1/metron/j1.h", "start": 74243, "end": 78263}, {"filename": "/examples/pong/README.md", "start": 78263, "end": 78323}, {"filename": "/examples/pong/main.cpp", "start": 78323, "end": 80269}, {"filename": "/examples/pong/main_vl.cpp", "start": 80269, "end": 80420}, {"filename": "/examples/pong/metron/pong.h", "start": 80420, "end": 84284}, {"filename": "/examples/pong/reference/README.md", "start": 84284, "end": 84344}, {"filename": "/examples/rvsimple/README.md", "start": 84344, "end": 84423}, {"filename": "/examples/rvsimple/main.cpp", "start": 84423, "end": 87189}, {"filename": "/examples/rvsimple/main_ref_vl.cpp", "start": 87189, "end": 90091}, {"filename": "/examples/rvsimple/main_vl.cpp", "start": 90091, "end": 93252}, {"filename": "/examples/rvsimple/metron/adder.h", "start": 93252, "end": 93752}, {"filename": "/examples/rvsimple/metron/alu.h", "start": 93752, "end": 95213}, {"filename": "/examples/rvsimple/metron/alu_control.h", "start": 95213, "end": 97818}, {"filename": "/examples/rvsimple/metron/config.h", "start": 97818, "end": 99033}, {"filename": "/examples/rvsimple/metron/constants.h", "start": 99033, "end": 104752}, {"filename": "/examples/rvsimple/metron/control_transfer.h", "start": 104752, "end": 105862}, {"filename": "/examples/rvsimple/metron/data_memory_interface.h", "start": 105862, "end": 107795}, {"filename": "/examples/rvsimple/metron/example_data_memory.h", "start": 107795, "end": 109022}, {"filename": "/examples/rvsimple/metron/example_data_memory_bus.h", "start": 109022, "end": 110266}, {"filename": "/examples/rvsimple/metron/example_text_memory.h", "start": 110266, "end": 110935}, {"filename": "/examples/rvsimple/metron/example_text_memory_bus.h", "start": 110935, "end": 111899}, {"filename": "/examples/rvsimple/metron/immediate_generator.h", "start": 111899, "end": 114017}, {"filename": "/examples/rvsimple/metron/instruction_decoder.h", "start": 114017, "end": 114781}, {"filename": "/examples/rvsimple/metron/multiplexer2.h", "start": 114781, "end": 115461}, {"filename": "/examples/rvsimple/metron/multiplexer4.h", "start": 115461, "end": 116277}, {"filename": "/examples/rvsimple/metron/multiplexer8.h", "start": 116277, "end": 117276}, {"filename": "/examples/rvsimple/metron/regfile.h", "start": 117276, "end": 118247}, {"filename": "/examples/rvsimple/metron/register.h", "start": 118247, "end": 118932}, {"filename": "/examples/rvsimple/metron/riscv_core.h", "start": 118932, "end": 121975}, {"filename": "/examples/rvsimple/metron/singlecycle_control.h", "start": 121975, "end": 127549}, {"filename": "/examples/rvsimple/metron/singlecycle_ctlpath.h", "start": 127549, "end": 130042}, {"filename": "/examples/rvsimple/metron/singlecycle_datapath.h", "start": 130042, "end": 134769}, {"filename": "/examples/rvsimple/metron/toplevel.h", "start": 134769, "end": 136757}, {"filename": "/examples/scratch.h", "start": 136757, "end": 137253}, {"filename": "/examples/tutorial/adder.h", "start": 137253, "end": 137433}, {"filename": "/examples/tutorial/bit_twiddling.h", "start": 137433, "end": 138414}, {"filename": "/examples/tutorial/blockram.h", "start": 138414, "end": 138931}, {"filename": "/examples/tutorial/checksum.h", "start": 138931, "end": 139654}, {"filename": "/examples/tutorial/clocked_adder.h", "start": 139654, "end": 140190}, {"filename": "/examples/tutorial/counter.h", "start": 140190, "end": 140339}, {"filename": "/examples/tutorial/declaration_order.h", "start": 140339, "end": 141118}, {"filename": "/examples/tutorial/functions_and_tasks.h", "start": 141118, "end": 142536}, {"filename": "/examples/tutorial/nonblocking.h", "start": 142536, "end": 142658}, {"filename": "/examples/tutorial/submodules.h", "start": 142658, "end": 143775}, {"filename": "/examples/tutorial/templates.h", "start": 143775, "end": 144246}, {"filename": "/examples/tutorial/tutorial2.h", "start": 144246, "end": 144314}, {"filename": "/examples/tutorial/tutorial3.h", "start": 144314, "end": 144355}, {"filename": "/examples/tutorial/tutorial4.h", "start": 144355, "end": 144396}, {"filename": "/examples/tutorial/tutorial5.h", "start": 144396, "end": 144437}, {"filename": "/examples/tutorial/vga.h", "start": 144437, "end": 145584}, {"filename": "/examples/uart/README.md", "start": 145584, "end": 145828}, {"filename": "/examples/uart/main.cpp", "start": 145828, "end": 147127}, {"filename": "/examples/uart/main_vl.cpp", "start": 147127, "end": 149663}, {"filename": "/examples/uart/metron/uart_hello.h", "start": 149663, "end": 152243}, {"filename": "/examples/uart/metron/uart_rx.h", "start": 152243, "end": 154804}, {"filename": "/examples/uart/metron/uart_top.h", "start": 154804, "end": 156568}, {"filename": "/examples/uart/metron/uart_tx.h", "start": 156568, "end": 159587}, {"filename": "/tests/metron_bad/README.md", "start": 159587, "end": 159784}, {"filename": "/tests/metron_bad/basic_reg_rwr.h", "start": 159784, "end": 160080}, {"filename": "/tests/metron_bad/basic_sig_wrw.h", "start": 160080, "end": 160328}, {"filename": "/tests/metron_bad/bowtied_signals.h", "start": 160328, "end": 160571}, {"filename": "/tests/metron_bad/case_without_break.h", "start": 160571, "end": 161166}, {"filename": "/tests/metron_bad/func_writes_sig_and_reg.h", "start": 161166, "end": 161393}, {"filename": "/tests/metron_bad/if_with_no_compound.h", "start": 161393, "end": 161803}, {"filename": "/tests/metron_bad/mid_block_break.h", "start": 161803, "end": 162343}, {"filename": "/tests/metron_bad/mid_block_return.h", "start": 162343, "end": 162654}, {"filename": "/tests/metron_bad/multiple_submod_function_bindings.h", "start": 162654, "end": 163100}, {"filename": "/tests/metron_bad/multiple_tock_returns.h", "start": 163100, "end": 163360}, {"filename": "/tests/metron_bad/tick_with_return_value.h", "start": 163360, "end": 163632}, {"filename": "/tests/metron_bad/too_many_breaks.h", "start": 163632, "end": 164139}, {"filename": "/tests/metron_good/README.md", "start": 164139, "end": 164220}, {"filename": "/tests/metron_good/all_func_types.h", "start": 164220, "end": 165879}, {"filename": "/tests/metron_good/basic_constructor.h", "start": 165879, "end": 166286}, {"filename": "/tests/metron_good/basic_function.h", "start": 166286, "end": 166565}, {"filename": "/tests/metron_good/basic_increment.h", "start": 166565, "end": 166920}, {"filename": "/tests/metron_good/basic_inputs.h", "start": 166920, "end": 167215}, {"filename": "/tests/metron_good/basic_literals.h", "start": 167215, "end": 167827}, {"filename": "/tests/metron_good/basic_localparam.h", "start": 167827, "end": 168073}, {"filename": "/tests/metron_good/basic_output.h", "start": 168073, "end": 168334}, {"filename": "/tests/metron_good/basic_param.h", "start": 168334, "end": 168593}, {"filename": "/tests/metron_good/basic_public_reg.h", "start": 168593, "end": 168824}, {"filename": "/tests/metron_good/basic_public_sig.h", "start": 168824, "end": 169004}, {"filename": "/tests/metron_good/basic_reg_rww.h", "start": 169004, "end": 169267}, {"filename": "/tests/metron_good/basic_sig_wwr.h", "start": 169267, "end": 169487}, {"filename": "/tests/metron_good/basic_submod.h", "start": 169487, "end": 169776}, {"filename": "/tests/metron_good/basic_submod_param.h", "start": 169776, "end": 170131}, {"filename": "/tests/metron_good/basic_submod_public_reg.h", "start": 170131, "end": 170507}, {"filename": "/tests/metron_good/basic_switch.h", "start": 170507, "end": 170984}, {"filename": "/tests/metron_good/basic_task.h", "start": 170984, "end": 171318}, {"filename": "/tests/metron_good/basic_template.h", "start": 171318, "end": 171804}, {"filename": "/tests/metron_good/basic_tock_with_return.h", "start": 171804, "end": 171963}, {"filename": "/tests/metron_good/bit_casts.h", "start": 171963, "end": 177936}, {"filename": "/tests/metron_good/bit_concat.h", "start": 177936, "end": 178363}, {"filename": "/tests/metron_good/bit_dup.h", "start": 178363, "end": 179022}, {"filename": "/tests/metron_good/both_tock_and_tick_use_tasks_and_funcs.h", "start": 179022, "end": 179860}, {"filename": "/tests/metron_good/builtins.h", "start": 179860, "end": 180191}, {"filename": "/tests/metron_good/call_tick_from_tock.h", "start": 180191, "end": 180498}, {"filename": "/tests/metron_good/case_with_fallthrough.h", "start": 180498, "end": 181052}, {"filename": "/tests/metron_good/constructor_arg_passing.h", "start": 181052, "end": 181945}, {"filename": "/tests/metron_good/constructor_args.h", "start": 181945, "end": 182453}, {"filename": "/tests/metron_good/defines.h", "start": 182453, "end": 182767}, {"filename": "/tests/metron_good/dontcare.h", "start": 182767, "end": 183047}, {"filename": "/tests/metron_good/enum_simple.h", "start": 183047, "end": 184415}, {"filename": "/tests/metron_good/for_loops.h", "start": 184415, "end": 184735}, {"filename": "/tests/metron_good/good_order.h", "start": 184735, "end": 185031}, {"filename": "/tests/metron_good/if_with_compound.h", "start": 185031, "end": 185442}, {"filename": "/tests/metron_good/include_guards.h", "start": 185442, "end": 185639}, {"filename": "/tests/metron_good/init_chain.h", "start": 185639, "end": 186347}, {"filename": "/tests/metron_good/initialize_struct_to_zero.h", "start": 186347, "end": 186634}, {"filename": "/tests/metron_good/input_signals.h", "start": 186634, "end": 187296}, {"filename": "/tests/metron_good/local_localparam.h", "start": 187296, "end": 187474}, {"filename": "/tests/metron_good/magic_comments.h", "start": 187474, "end": 187777}, {"filename": "/tests/metron_good/matching_port_and_arg_names.h", "start": 187777, "end": 188091}, {"filename": "/tests/metron_good/minimal.h", "start": 188091, "end": 188166}, {"filename": "/tests/metron_good/multi_tick.h", "start": 188166, "end": 188532}, {"filename": "/tests/metron_good/namespaces.h", "start": 188532, "end": 188882}, {"filename": "/tests/metron_good/nested_structs.h", "start": 188882, "end": 189297}, {"filename": "/tests/metron_good/nested_submod_calls.h", "start": 189297, "end": 189842}, {"filename": "/tests/metron_good/oneliners.h", "start": 189842, "end": 190105}, {"filename": "/tests/metron_good/plus_equals.h", "start": 190105, "end": 190529}, {"filename": "/tests/metron_good/private_getter.h", "start": 190529, "end": 190753}, {"filename": "/tests/metron_good/structs.h", "start": 190753, "end": 190972}, {"filename": "/tests/metron_good/structs_as_args.h", "start": 190972, "end": 191508}, {"filename": "/tests/metron_good/structs_as_ports.h", "start": 191508, "end": 194052}, {"filename": "/tests/metron_good/submod_bindings.h", "start": 194052, "end": 194806}, {"filename": "/tests/metron_good/tock_task.h", "start": 194806, "end": 195162}, {"filename": "/tests/metron_good/trivial_adder.h", "start": 195162, "end": 195327}, {"filename": "/tests/metron_good/utf8-mod.bom.h", "start": 195327, "end": 195986}, {"filename": "/tests/metron_good/utf8-mod.h", "start": 195986, "end": 196645}], "remote_package_size": 196645});

  })();
