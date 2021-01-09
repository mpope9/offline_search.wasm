export class OfflineSearch {

  constructor(module) { 
    this.indexSize = 0;
    this.Module = module;
  }

  async init() {
    await this.Module.then(function(module) {
      module._initialize_index();
    });
  }

  async search(input) {

    var idxSize = this.indexSize;

    var output = await this.Module.then(function(module) {

      var outputString = '';
      // Allocate space for input string.
      var inputInts = module.intArrayFromString(input, false);
      var inputPtr = module.allocate(inputInts, module.ALLOC_NORMAL);

      // Allocate space for search init output.
      var initOutputPtr = module._malloc(idxSize * module.HEAP32.BYTES_PER_ELEMENT);

      // From the inputPtr, write index mappings to initOutputPtr
      var finalizeOutputSize = module._initiate_search(inputPtr, initOutputPtr);

      // No matches, or stop word detected.
      if(finalizeOutputSize == 0)
      {
        module._free(finalizeOutputPtr);
        module._free(initOutputPtr);
        return outputString;
      }

      // Allocate memory for final output string.
      var finalizeOutputPtr = module._malloc(finalizeOutputSize * module.HEAP8.BYTES_PER_ELEMENT);

      // From the index mappings, get the urls and titles. Write to finalizeOutputPtr.
      module._finalize_search(initOutputPtr, finalizeOutputPtr);

      var i;
      for(i = 0; i < finalizeOutputSize; ++i) {
        outputString += String.fromCharCode(module.getValue(finalizeOutputPtr + i));
      }

      // Free up some memory.
      module._free(finalizeOutputPtr);
      module._free(initOutputPtr);
      module._free(inputPtr);

      return outputString;
    });

    output = output.split("||");
    output.pop();
    return output;
  }

  async free() {
    await this.Module.then(function(module) {
      module._free_index();
    });
  }
}
