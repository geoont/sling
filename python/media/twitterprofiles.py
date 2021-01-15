# Copyright 2020 Ringgaard Research ApS
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http:#www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""Get profile picture from Twitter profiles"""

import json
import requests
import urllib

import sling
import sling.flags as flags
import sling.log as log
import sling.task.data as data
from sling.task.workflow import *

flags.define("--twitterdb",
             help="database for storing Twitter profiles",
             default="http://localhost:7070/twitter",
             metavar="DBURL")

bad_images = set([
  "http://pbs.twimg.com/profile_images/1302121919014207490/KaYYEC8b.jpg"
])

# Task for extracting images from Twitter profiles.
class TwitterExtract:
  def run(self, task):
    # Get parameters.
    twitterdb = task.input("twitterdb").name

    # Load knowledge base.
    log.info("Load knowledge base")
    kb = sling.Store()
    kb.load(task.input("kb").name)

    p_id = kb["id"]
    p_is = kb["is"]
    p_twitter = kb["P2002"]
    p_image = kb["P18"]
    p_media = kb["media"]
    p_stated_in = kb["P248"]
    n_twitter = kb["Q918"]

    kb.freeze()

    # Open output file.
    fout = open(task.output("output").name, "w")

    # Find all items with twitter usernames.
    dbsession = requests.session()
    for item in kb:
      # Find twitter username for item.
      task.increment("items")
      twitter = item[p_twitter]
      if twitter is None: continue
      username = kb.resolve(twitter)
      task.increment("twitter_users")

      # Fetch twitter profile from database.
      dburl = twitterdb + "/" + urllib.parse.quote(username)
      r = dbsession.get(dburl)
      if r.status_code == 404:
        task.increment("unknown_users")
        continue
      r.raise_for_status()
      profile = r.json()

      # Ignore if twitter profile does not exist.
      if "error" in profile:
        task.increment("deleted_users")
        continue

      # Ignore if there is no profile image.
      if profile["default_profile_image"]:
        task.increment("missing_profile_images")
        continue

      # Get profile image url.
      imageurl = profile["profile_image_url"]

      # Get url for original image url by removing "_normal".
      imageurl = ''.join(imageurl.rsplit("_normal", 1))

      # Ignore known bad images.
      if imageurl in bad_images:
        task.increment("bad_profile_images")
        continue

      # Create item frame with twitter profile.
      store = sling.Store(kb)
      image = store.frame([(p_is, imageurl), (p_stated_in, n_twitter)])
      frame = store.frame([(p_id, item.id), (p_media, image)])
      fout.write(frame.data(utf8=True))
      fout.write("\n")

      task.increment("profile_images")
      if p_image not in item: task.increment("imaged_items")

    fout.close()

register_task("twitter-extract", TwitterExtract)

class TwitterWorkflow:
  def __init__(self, name=None):
    self.wf = Workflow(name)
    self.data = data.Datasets(self.wf)

  def twitterdb(self):
    """Resource for Twitter database."""
    return self.wf.resource(flags.arg.twitterdb, format="url/json")

  def twitter_frames(self):
    """Resource for twitter frames."""
    return self.wf.resource("twitter-media.sling",
                            dir=flags.arg.workdir + "/media",
                            format="text/frames")

  def extract_twitter(self):
    extractor = self.wf.task("twitter-extract")
    extractor.attach_input("kb", self.data.knowledge_base())
    extractor.attach_input("twitterdb", self.twitterdb())
    extractor.attach_output("output", self.twitter_frames())

# Commands.

def twitter_profiles():
  log.info("Extract twitter profiles")
  wf = TwitterWorkflow("twitter-profiles")
  wf.extract_twitter()
  run(wf.wf)
