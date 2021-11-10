// Copyright 2020 Ringgaard Research ApS
// Licensed under the Apache License, Version 2

// SLING case plug-in for adding photo albums to topics.

import {store, settings} from "/case/app/global.js";

const n_is = store.lookup("is");
const n_media = store.lookup("media");
const n_has_quality = store.lookup("P1552");
const n_not_safe_for_work = store.lookup("Q2716583");

export default class AlbumPlugin {
  async process(action, url, context) {
    let topic = context.topic;
    if (!topic) return false;
    console.log(`Add album ${url} to topic ${context.topic.id}`);

    // Get URLs for album from album service.
    let r = await fetch(`/case/service/albums?url=${encodeURIComponent(url)}`);
    let profile = await store.parse(r);

    // Get existing photos to remove duplicates.
    let photos = new Set();
    for (let media of topic.all(n_media)) photos.add(store.resolve(media));

    // Add media for album.
    let num_added = 0;
    for (let media of profile.all(n_media)) {
      let url = store.resolve(media);
      if (photos.has(url)) {
        console.log("skip exising image", url);
        continue;
      }
      photos.add(url);
      topic.add(n_media, media);
      num_added++;
    }

    console.log(`${num_added} images added`);
    return num_added > 0;
  }
};

