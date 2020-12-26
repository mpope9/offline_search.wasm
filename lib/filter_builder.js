/* !
 * filter_builder.js
 * Copyright (C) @YEAR Matthew Pope
 */

const Module = require('../_build/xor_builder.js');
const fs = require('fs');

const fsRoot = '_build/filters/';

// Test Blog Post
var testPostName = 'test_post';
var testPost = `
You probably already know how integral the process of blogging is to the success of your marketing efforts. Which is why it goes without saying it's exceptionally important to learn how to effectively start and manage a blog in a way that supports your business.

Without a blog, you'll find yourself experiencing a number of problems such as poor search engine optimization (SEO), lack of promotional content for social, little clout with your leads and customers, and fewer pages to share your lead-generating calls-to-action (CTAs) on.

So why, oh why, do so many marketers I talk to still have a laundry list of excuses for why they can't maintain a blog?

Maybe because, unless you enjoy writing, business blogging might seem uninteresting, time consuming, and difficult.

Well, the time for excuses is over and this guide is here to help you understand why. We'll cover how to write and manage your business's blog as well as provide helpful templates to simplify your blogging efforts.

Before you write a blog, make sure you know the answers to questions like, "Why would someone keep reading this entire blog post?" and "What makes our audience come back for more?"

To start, a good blog post is interesting and educational. Blogs should answer questions and help readers resolve a challenge they're experiencing — and you have to do so in an interesting way.

It's not enough just to answer someone's questions — you also have to provide actionable steps while being engaging. For instance, your introduction should hook the reader and make them want to continue reading your post. Then, use examples to keep your readers interested in what you have to say.

Remember, a good blog post is interesting to read and provides educational content to audience members
`

Module().then(function(mod) {
  index_post(mod, testPost, testPostName);
});

function index_post(mod, post, name) {

  var fileNameInts = mod.intArrayFromString(name + '.bin', false);
  var fileNamePtr = mod.allocate(fileNameInts, mod.ALLOC_NORMAL);

  var postInts = mod.intArrayFromString(post);
  var postPtr = mod.allocate(postInts, mod.ALLOC_NORMAL);

  var result = mod._build_xor(postPtr, post.length, fileNamePtr);

  mod._free(fileNamePtr);
  mod._free(postPtr);
}
