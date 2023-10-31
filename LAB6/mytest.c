#include "myalloc.c"

int main(){
  int page_size = getpagesize();

  // Test: Only split if the process of splitting would leaves 
  // enough room for another chunk.
  myinit(page_size);
  void* buff = myalloc(64);
  //This should leave 10 bytes remaining in the arena
  size_t size = page_size - 64 - (sizeof(node_t) * 2) - 10;
  void* buff2 = myalloc(size);

  node_t* header2 = (node_t *)(buff2 - sizeof(node_t));

  printf("Expected: %ld , Got %ld\n", (size + 10), header2->size);

}