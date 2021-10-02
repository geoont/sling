// Copyright 2020 Ringgaard Research ApS
// Licensed under the Apache License, Version 2

import {Component} from "/common/lib/component.js";
import {MdCard, MdIcon, MdDialog, StdDialog} from "/common/lib/material.js";
import {store, settings} from "./global.js";

const n_id = store.id;
const n_is = store.is;
const n_name = store.lookup("name");
const n_caseno = store.lookup("caseno");
const n_main = store.lookup("main");
const n_topics = store.lookup("topics");
const n_folders = store.lookup("folders");
const n_next = store.lookup("next");
const n_sling_case_no = store.lookup("PCASE");

//-----------------------------------------------------------------------------
// Case Editor
//-----------------------------------------------------------------------------

class CaseEditor extends Component {
  onconnected() {
    this.app = this.match("#app");
    this.bind("#menu", "click", e => this.onmenu(e));
    this.bind("#home", "click", e => this.close());
    this.bind("#save", "click", e => this.onsave(e));
    this.bind("#newfolder", "click", e => this.onnewfolder(e));

    document.addEventListener("keydown", e => this.onkeydown(e));
    window.addEventListener("beforeunload", e => this.onbeforeunload(e));
  }

  onbeforeunload(e) {
    // Notify about unsaved changes.
    if (this.dirty) {
      e.preventDefault();
      e.returnValue = "";
    }
  }

  onkeydown(e) {
    if (e.ctrlKey && e.key === 's') {
      e.preventDefault();
      this.onsave(e);
    }
  }

  onnewfolder(e) {
    let dialog = new NewFolderDialog();
    dialog.show().then(result => {
      if (result) {
        this.add_folder(result);
      }
    });
  }

  onmenu(e) {
    this.find("md-drawer").toogle();
  }

  onsave(e) {
    if (this.dirty) {
      this.match("#app").save_case(this.casefile);
      this.mark_clean();
    }
  }

  onupdated() {
    if (!this.state) return;
    this.mark_clean();

    this.casefile = this.state;
    this.main = this.casefile.get(n_main);
    this.topics = this.casefile.get(n_topics);
    this.folders = this.casefile.get(n_folders);
    this.folder = this.casefile.get(n_folders).value(0);

    this.find("#caseno").update(this.caseno().toString());
    this.find("folder-list").update(this.folders);
    this.find("topic-list").update(this.folder);
    this.find("md-drawer").update(true);
  }

  caseno() {
    return this.casefile.get(n_caseno);
  }

  name() {
    return this.main.get(n_name);
  }

  next_topic() {
    let next = this.casefile.get(n_next);
    this.casefile.set(n_next, next + 1);
    return next;
  }

  mark_clean() {
    this.dirty = false;
    this.find("#save").disable();
  }

  mark_dirty() {
    this.dirty = true;
    this.find("#save").enable();
  }

  close() {
    if (this.dirty) {
      let msg = `Changes to case #${this.caseno()} has not been saved.`;
      let buttons = {
        "Close without saving": "close",
        "Cancel": "cancel",
        "Save": "save",
      }
      StdDialog.choose("Discard changes?", msg, buttons).then(result => {
        if (result == "save") {
          this.match("#app").save_case(this.casefile);
          this.mark_clean();
          app.show_manager();
        } else if (result == "close") {
          this.mark_clean();
          app.show_manager();
        }
      });

    } else {
      app.show_manager();
    }
  }

  add_folder(name) {
    this.folder = new Array();
    this.folders.add(name, this.folder);
    this.find("folder-list").update(this.folders);
    this.find("topic-list").update(this.folder);
    this.mark_dirty();
  }

  show_folder(folder) {
    this.folder = folder;
    console.log("show folder", this.folder);
    this.find("folder-list").update(this.folders);
    this.find("topic-list").update(this.folder);
  }

  add_topic(itemid, name) {
    // Create new topic.
    let topicid = this.next_topic();
    let topic = store.frame(`t/${this.caseno()}/${topicid}`);
    if (itemid) topic.add(n_is, store.lookup(itemid));
    if (name) topic.add(n_name, name);

    // Add topic to current folder.
    this.topics.push(topic);
    this.folder.push(topic);

    // Update topic list.
    let topic_list = this.find("topic-list");
    topic_list.update(this.folder);
    topic_list.scroll_to(topic);
    this.mark_dirty();
  }

  delete_topic(topic) {
    // Do not delete main topic.
    if (topic == this.casefile.get(n_main)) return;

    // Delete topic from case and current folder.
    this.topics.splice(this.topics.indexOf(topic), 1);
    this.folder.splice(this.folder.indexOf(topic), 1);
    this.mark_dirty();

    // Update topic list.
    this.find("topic-list").update(this.folder);
  }

  move_up(topic) {
    let pos = this.folder.indexOf(topic);
    if (pos == -1) return;
    if (pos == 0 || this.folder.length == 1) return;

    // Swap with previous topic.
    let tmp = this.folder[pos];
    this.folder[pos] = this.folder[pos - 1];
    this.folder[pos - 1] = tmp;
    this.mark_dirty();

    // Update topic list.
    this.find("topic-list").update(this.folder);
  }

  move_down(topic) {
    let pos = this.folder.indexOf(topic);
    if (pos == -1) return;
    if (pos == this.folder.length - 1 || this.folder.length == 1) return;

    // Swap with next topic.
    let tmp = this.folder[pos];
    this.folder[pos] = this.folder[pos + 1];
    this.folder[pos + 1] = tmp;
    this.mark_dirty();

    // Update topic list.
    this.find("topic-list").update(this.folder);
  }

  prerender() {
    return `
      <md-column-layout>
        <md-toolbar>
          <md-icon-button id="menu" icon="menu"></md-icon-button>
          <md-toolbar-logo></md-toolbar-logo>
          <div id="title">Case #<md-text id="caseno"></md-text></div>
          <topic-search-box id="search"></topic-search-box>
          <md-spacer></md-spacer>
          <md-icon-button id="save" icon="save"></md-icon-button>
        </md-toolbar>

        <md-row-layout>
          <md-drawer>
            <div id="home">
              <md-icon-button icon="home"></md-icon-button>
              SLING Cases Home
            </div>
            <div id="folders-top">
              Folders
              <md-spacer></md-spacer>
              <md-icon-button id="newfolder" icon="create_new_folder">
              </md-icon-button>
            </div>
            <folder-list></folder-list>
          </md-drawer>
          <md-content>
            <topic-list></topic-list>
          </md-content>
        </md-row-layout>
      </md-column-layout>
    `;
  }
  static stylesheet() {
    return `
      $ md-toolbar {
        padding-left: 2px;
      }
      $ #title {
        white-space: nowrap;
      }
      $ md-row-layout {
        overflow: auto;
        height: 100%;
      }
      $ md-drawer md-icon {
        color: #808080;
      }
      $ md-drawer md-icon-button {
        color: #808080;
      }
      $ topic-list md-icon {
        color: #808080;
      }
      $ topic-list md-icon-button {
        color: #808080;
        fill: #808080;
      }
      $ md-drawer {
        padding: 3px;
        box-sizing: border-box;
      }
      $ #home {
        display: flex;
        align-items: center;
        padding-right: 16px;
        font-weight: bold;
        white-space: nowrap;
        cursor: pointer;
      }
      $ #folders-top {
        display: flex;
        align-items: center;
        font-size: 16px;
        font-weight: bold;
        margin-left: 6px;
        border-bottom: 1px solid #808080;
      }
    `;
  }
}

Component.register(CaseEditor);

class TopicSearchBox extends Component {
  onconnected() {
    this.bind("md-search", "query", e => this.onquery(e));
    this.bind("md-search", "item", e => this.onitem(e));
    this.bind("md-search", "enter", e => this.onenter(e));
  }

  onquery(e) {
    let detail = e.detail
    let target = e.target;
    let params = "fmt=cjson";
    let query = detail.trim();
    if (query.endsWith(".")) {
      params += "&fullmatch=1";
      query = query.slice(0, -1);
    }
    params += `&q=${encodeURIComponent(query)}`;

    this.itemnames = new Map();
    fetch(`${settings.kbservice}/kb/query?${params}`)
    .then(response => response.json())
    .then((data) => {
      let items = [];
      for (let item of data.matches) {
        let elem = document.createElement("md-search-item");
        elem.setAttribute("name", item.text);
        elem.setAttribute("value", item.ref);

        let title = document.createElement("span");
        title.className = "item-title";
        title.appendChild(document.createTextNode(item.text));
        elem.appendChild(title);

        if (item.description) {
          let desciption = document.createElement("span");
          desciption.className = "item-description";
          desciption.appendChild(document.createTextNode(item.description));
          elem.appendChild(desciption);
        }

        items.push(elem);
        this.itemnames[item.ref] = item.text;
      }
      target.populate(detail, items);
    })
    .catch(error => {
      console.log("Query error", query, error.message, error.stack);
      StdDialog.error(error.message);
      target.populate(detail, null);
    });
  }

  onenter(e) {
    let name = e.detail;
    this.match("#editor").add_topic(null, name);
  }

  onitem(e) {
    let topic = e.detail;
    let name = this.itemnames[topic];
    this.match("#editor").add_topic(topic, name);
  }

  query() {
    return this.find("md-search").query();
  }

  clear() {
    return this.find("md-search").clear();
  }

  render() {
    return `
      <form>
        <md-search
          placeholder="Search for topic..."
          min-length=2
          autofocus>
        </md-search>
      </form>
    `;
  }

  static stylesheet() {
    return `
      $ {
        display: block;
        width: 100%;
        max-width: 800px;
        padding-left: 10px;
        padding-right: 3px;
      }

      $ form {
        display: flex;
        width: 100%;
      }

      $ .item-title {
        font-weight: bold;
        display: block;
        padding: 2px 10px 2px 10px;
      }

      $ .item-description {
        display: block;
        padding: 0px 10px 0px 10px;
      }
    `;
  }
}

Component.register(TopicSearchBox);

class FolderList extends Component {
  render() {
    let folders = this.state;
    if (!folders) return;
    let current = this.match("#editor").folder;
    let h = [];
    for (let [name, folder] of folders) {
      h.push(new CaseFolder({name, folder, marked: folder == current}));
    }
    return h;
  }
}

Component.register(FolderList);

class CaseFolder extends Component {
  onconnected() {
    this.bind(null, "click", e => this.onclick(e));
  }

  onclick(e) {
    this.match("#editor").show_folder(this.state.folder);
  }

  render() {
    return `
      <md-icon icon="folder"></md-icon>
      <div ${this.state.marked ? 'class="current"' : ''}>
        ${Component.escape(this.state.name)}
      </div>
      <md-spacer></md-spacer>
      <md-icon-button icon="more_vert"></md-icon-button>
    `;
  }

  static stylesheet() {
    return `
      $ {
        display: flex;
        align-items: center;
        padding-left: 6px;
        white-space: nowrap;
        cursor: pointer;
        height: 30px;
        border-radius: 15px;
        border: 0;
      }
      $ md-icon-button button {
        height: 30px;
        width: 30px;
        border-radius: 15px;
      }
      $:hover {
        background-color: #eeeeee;
      }
      $ div {
        padding-left: 6px;
      }
      $ div.current {
        font-weight: bold;
        padding-left: 6px;
      }
    `;
  }
}

Component.register(CaseFolder);

class NewFolderDialog extends MdDialog {
  submit() {
    this.close(this.find("#name").value.trim());
  }

  render() {
    let p = this.state;
    return `
      <md-dialog-top>Create new case folder</md-dialog-top>
      <div id="content">
        <md-text-field
          id="name"
          value="${Component.escape(this.state)}"
          label="Folder name">
        </md-text-field>
      </div>
      <md-dialog-bottom>
        <button id="cancel">Cancel</button>
        <button id="submit">Create</button>
      </md-dialog-bottom>
    `;
  }

  static stylesheet() {
    return MdDialog.stylesheet() + `
      $ #content {
        display: flex;
        flex-direction: column;
        row-gap: 16px;
      }
      $ #name {
        width: 300px;
      }
    `;
  }
}

Component.register(NewFolderDialog);

class TopicList extends Component {
  scroll_to(topic) {
    this.card(topic).scrollIntoView();
  }

  card(topic) {
    for (let i = 0; i < this.children.length; i++) {
      let card = this.children[i];
      if (card.state == topic) return card;
    }
    return null;
  }

  render() {
    let topics = this.state;
    if (!topics) return;
    let h = [];
    for (let topic of topics) {
      h.push(new TopicCard(topic));
    }
    return h;
  }

  static stylesheet() {
    return `
    `;
  }
}

Component.register(TopicList);

class TopicCard extends MdCard {
  onconnected() {
    this.bind("#delete", "click", e => this.ondelete(e));
    this.bind("#moveup", "click", e => this.onmoveup(e));
    this.bind("#movedown", "click", e => this.onmovedown(e));
  }

  ondelete(e) {
    let topic = this.state;
    let message = `Delete topic '${topic.get(n_name)}'?`;
    StdDialog.confirm("Delete topic", message, "Delete").then(result => {
      if (result) {
        this.match("#editor").delete_topic(topic);
      }
    });
  }

  onmoveup(e) {
    this.match("#editor").move_up(this.state);
  }

  onmovedown(e) {
    this.match("#editor").move_down(this.state);
  }

  render() {
    let topic = this.state;
    if (!topic) return;

    return `
      <md-card-toolbar>
        <div>
          <div id="name">${topic.get(n_name)}</div>
          <div id="id">${topic.get(n_id)}</div>
        </div>
        <md-spacer></md-spacer>
        <div id="actions">
          <md-icon-button id="edit" icon="edit"></md-icon-button>
          <md-icon-button id="moveup" icon="move-up"></md-icon-button>
          <md-icon-button id="movedown" icon="move-down"></md-icon-button>
          <md-icon-button id="delete" icon="delete"></md-icon-button>
        </div>
      </md-card-toolbar>
      <pre>${Component.escape(topic.text(true))}</pre>
    `;
  }

  static stylesheet() {
    return MdCard.stylesheet() + `
      $ #name {
        display: block;
        font-size: 24px;
      }
      $ #id {
        display: block;
        font-size: 13px;
        color: #808080;
        text-decoration: none;
        width: fit-content;
        outline: none;
      }
      $ #actions {
        display: flex;
      }
    `;
  }
}

Component.register(TopicCard);

MdIcon.custom("move-down", `
<svg width="24" height="24" viewBox="0 0 32 32">
  <g><polygon points="30,16 22,16 22,2 10,2 10,16 2,16 16,30"/></g>
</svg>
`);

MdIcon.custom("move-up", `
<svg width="24" height="24" viewBox="0 0 32 32">
  <g><polygon points="30,14 22,14 22,28 10,28 10,14 2,14 16,0"/></g>
</svg>
`);

