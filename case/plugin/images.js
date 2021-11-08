// Copyright 2020 Ringgaard Research ApS
// Licensed under the Apache License, Version 2

// SLING case plug-in for adding images to topics.

import {store, settings} from "/case/app/global.js";

const n_media = store.lookup("media");

export default class ImagePlugin {
  process(url, context) {
    if (context.topic) {
      console.log(`add image ${url} to topic ${context.topic.id}`);
      context.topic.add(n_media, url);
      return true;
    } else {
      return false;
    }
  }
};

