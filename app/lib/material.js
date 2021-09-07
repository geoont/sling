// Material Design web components.

import {Component, stylesheet} from "./component.js";

//-----------------------------------------------------------------------------
// Global styles
//-----------------------------------------------------------------------------

stylesheet(`
@import url(https://fonts.googleapis.com/css?family=Roboto:400,400italic,500,500italic,700,700italic,900,900italic,300italic,300,100italic,100);

@font-face {
  font-family: 'Material Icons';
  font-style: normal;
  font-weight: 400;
  src: url(https://fonts.gstatic.com/s/materialicons/v55/flUhRq6tzZclQEJ-Vdg-IuiaDsNc.woff2) format('woff2');
}

html {
  width: 100%;
  height: 100%;
  min-height: 100%;
  position:relative;
}

body {
  font-family: Roboto,Helvetica,sans-serif;
  font-size: 14px;
  font-weight: 400;
  padding: 0;
  margin: 0;
  box-sizing: border-box;

  width: 100%;
  height: 100%;
  min-height: 100%;
  position: relative;
}
`);

//-----------------------------------------------------------------------------
// Layout
//-----------------------------------------------------------------------------

export class MdColumnLayout extends Component {
  static stylesheet() {
    return `
      $ {
        display: flex;
        flex-direction: column;
        margin: 0;
        height: 100%;
        min-height: 100%;
      }
    `;
  }
}

Component.register(MdColumnLayout);

export class MdRowLayout extends Component {
  static stylesheet() {
    return `
      $ {
        display: flex;
        flex-direction: row;
        margin: 0;
        width: 100%;
        min-width: 100%;
      }
    `;
  }
}

Component.register(MdRowLayout);

export class MdContent extends Component {
  static stylesheet() {
    return `
      $ {
        flex: 1;
        padding: 8px;
        display: block;
        overflow: auto;
        color: rgb(0,0,0);
        background-color: #eeeeee;

        position: relative;

        flex-basis: 0%;
        flex-grow: 1;
        flex-shrink: 1;
      }
    `;
  }
}

Component.register(MdContent);

export class MdSpacer extends Component {
  static stylesheet() {
    return `
      $ {
        display: block;
        flex: 1;
      }
    `;
  }
}

Component.register(MdSpacer);

//-----------------------------------------------------------------------------
// Modal
//-----------------------------------------------------------------------------

export class MdModal extends Component {
  open(state) {
    document.body.appendChild(this);
    this.tabIndex = -1;
    this.focus();
    if (this.onopen) this.onopen();
    this.update(state);
  }

  close(e) {
    if (this.onclose) this.onclose(e);
    document.body.removeChild(this);
  }

  static stylesheet() {
    return `
      $ {
        display: block;
        position: fixed;
        z-index: 100;
        left: 0;
        top: 0;
        width: 100%;
        height: 100%;
      }
    `;
  }
}

Component.register(MdModal);

//-----------------------------------------------------------------------------
// Dialog
//-----------------------------------------------------------------------------

export class MdDialog extends Component {
  show() {
    document.body.insertAdjacentHTML("beforeend", "<dialog></dialog>");
    this.dialog = document.body.lastChild;
    this.dialog.addEventListener("close", e => this.cancel());
    this.dialog.appendChild(this);

    if (this.onopen) this.onopen();
    this.dialog.showModal();
    this.bind(null, "keydown", e => { if (e.keyCode == 13) this.done(); });

    let promise = new Promise((resolve, reject) => { this.resolve = resolve; });
    return promise;
  }

  close(cancel) {
    if (this.onclose) this.onclose(cancel);
    document.body.removeChild(this.dialog);
    if (!cancel) this.resolve(this.state);
  }

  done() { this.close(false); }
  cancel() { this.close(true); }

  static stylesheet() {
    return `
      dialog {
        border-style: none;
        padding: 0px;
        border-radius: 5px;
        box-shadow: 0 14px 28px rgba(0,0,0,0.25), 0 10px 10px rgba(0,0,0,0.22);
      }

      $ {
        display: block;
        padding-left: 16px;
        padding-right: 16px;
      }
    `;
  }
}

Component.register(MdDialog);

export class MdDialogTop extends Component {
  static stylesheet() {
    return `
      $ {
        display: block;
        margin-top: 16px;
        margin-bottom: 16px;
        font-size: 1.25rem;
        line-height: 2rem;
        font-weight: 500;
        letter-spacing: .0125em;
      }
    `;
  }
}

Component.register(MdDialogTop);

export class MdDialogBottom extends Component {
  static stylesheet() {
    return `
      $ {
        display: flex;
        justify-content: flex-end;
        flex-shrink: 0;
        flex-wrap: wrap;
        padding-top: 8px;
        padding-bottom: 8px;
      }
      $ button {
        font: bold 14px Roboto,Helvetica,sans-serif;
        color: #00A0D6;
        background-color: #ffffff;
        border: none;
        border-radius: 4px;
        text-transform: uppercase;
        letter-spacing: 1.25px;
        text-align: right;
        padding: 8px;
        margin-left: 4px;
      }
      $ button:hover {
        background-color: #eeeeee;
      }
      $ button:active {
        background-color: #aaaaaa;
      }
    `;
  }
}

Component.register(MdDialogBottom);

//-----------------------------------------------------------------------------
// Toolbar
//-----------------------------------------------------------------------------

export class MdToolbar extends Component {
  static stylesheet() {
    return `
      $ {
        display: flex;
        flex-direction: row;
        align-items: center;
        background-color: #00A0D6;
        color: rgb(255,255,255);
        height: 56px;
        max-height: 56px;
        font-size: 20px;
        padding: 0px 16px;
        margin: 0;
        box-shadow: 0 1px 8px 0 rgba(0,0,0,.2),
                    0 3px 4px 0 rgba(0,0,0,.14),
                    0 3px 3px -2px rgba(0,0,0,.12);
        z-index: 2;
      }
    `;
  }
}

Component.register(MdToolbar);

//-----------------------------------------------------------------------------
// Tabs
//-----------------------------------------------------------------------------

export class MdTabs extends Component {
  constructor() {
    super();
    this.selected = this.find(this.getAttribute("selected"));
  }

  onconnected() {
    this.bind(null, "click", e => this.onclick(e));
    if (this.selected) this.select(this.selected);
  }

  onclick(e) {
    if (e.target != this) this.select(e.target);
  }

  select(tab) {
    if (!tab) return;
    if (this.selected) this.selected.classList.remove("selected");
    this.selected = tab;
    this.selected.classList.add("selected");
  }

  static stylesheet() {
    return `
      $ {
        height: 100%;
        margin: 5px;
        display: table;
        flex-direction: row;
        align-items: center;
        border-spacing: 3px;
      }

      $ .selected {
        border-bottom: 2px solid;
      }
    `;
  }
}

Component.register(MdTabs);

export class MdTab extends Component {
  static stylesheet() {
    return `
      $ {
        height: 100%;
        text-transform: uppercase;
        padding: 5px;
        text-align: center;
        display: table-cell;
        vertical-align: middle;
        font-size: 16px;
        cursor: pointer;
      }

      $:hover {
        background-color: rgba(0,0,0,0.07);
      }
    `;
  }
}

Component.register(MdTab);

//-----------------------------------------------------------------------------
// Card
//-----------------------------------------------------------------------------

export class MdCard extends Component {
  static stylesheet() {
    return `
      $ {
        display: block;
        background-color: rgb(255, 255, 255);
        box-shadow: rgba(0, 0, 0, 0.16) 0px 2px 4px 0px,
                    rgba(0, 0, 0, 0.23) 0px 2px 4px 0px;
        padding: 10px;
        margin: 5px;
      }
    `;
  }
}

Component.register(MdCard);

export class MdCardToolbar extends Component {
  static stylesheet() {
    return `
      $ {
        display: flex;
        flex-direction: row;
        align-items: top;
        background-color: #FFFFFF;
        color: #000000;
        font-size: 24px;
        margin-bottom: 10px;
      }
    `;
  }
}

Component.register(MdCardToolbar);

//-----------------------------------------------------------------------------
// Logo
//-----------------------------------------------------------------------------

const logo = "\
M257.94,57.87c-16,22.9-37.86,45.41-62.54,60a60.83,60.83,0,0,0,7.66-29.64,61.\
23,61.23,0,0,0-61.25-61.06h0c-49.88,0-96.7,40.07-126,80.91-2.86-3.22-5.58-6.\
53-8.19-9.86S2.44,91.58,0,88.27C32.81,43.72,86,0,141.73,0,184.65,0,226.22,25\
.65,257.94,57.87Zm18,20.32c-2.62-3.33-5.33-6.64-8.19-9.86-29.33,40.84-76.16,\
80.91-126,80.91h0a61,61,0,0,1-53.59-90.7c-24.68,14.63-46.59,37.15-62.55,60,3\
1.73,32.22,73.3,57.89,116.21,57.88,55.79,0,108.93-43.73,141.73-88.28C281,84.\
88,278.52,81.53,275.91,78.19Zm-134.48-37a46.87,46.87,0,1,0,46.88,46.87A46.87\
,46.87,0,0,0,141.43,41.18Z";

export class MdLogo extends Component {
  render() {
    return `
      <a href="/" tabindex="-1">
        <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 283.46 176.46">
          <g><path d="${logo}"/></g>
        </svg>
      </a>`;
  }

  static stylesheet() {
    return `
      $ svg {
        width: 100%;
      }
      $ path {
        fill: #00A0D6; }
      }
    `;
  }
}

Component.register(MdLogo);

export class MdToolbarLogo extends MdLogo {
  static stylesheet() {
    return `
      $ {
        margin: 5px 10px 0px 0px;
        outline: none;
      }
      $ svg {
        width: 50px;
      }
      $ path {
        fill: #FFFFFF; }
      }
    `;
  }
}

Component.register(MdToolbarLogo);

//-----------------------------------------------------------------------------
// Button
//-----------------------------------------------------------------------------

export class MdIconButton extends Component {
  constructor() {
    super();
    this.state = true;
  }

  visible() {
    return this.state;
  }

  render() {
    let attrs = [];
    if (this.props.disabled) attrs.push(' disabled');
    if (this.props.shortcut) attrs.push(` accesskey="${this.props.shortcut}"`);
    if (this.props.type) attrs.push(` type="${this.props.type}"`);
    return `<button ${attrs.join("")}><i>${this.props.icon}</i></button>`;
  }

  disable() {
    if (!this.props.disabled) {
      this.props.disabled = true;
      this.update();
    }
  }

  enable() {
    if (this.props.disabled) {
      this.props.disabled = false;
      this.update();
    }
  }

  static stylesheet() {
    return `
      $ button {
        border-radius: 50%;
        border: 0;
        height: 40px;
        width: 40px;
        background: transparent;
        user-select: none;
        cursor: pointer;
      }

      $ button:hover:enabled {
        background-color: rgba(0,0,0,0.07);
      }

      md-icon-button button:disabled {
        color: rgba(0,0,0,0.38);
        cursor: default;
      }

      md-toolbar $ button {
        color: rgb(255,255,255);
      }

      $ button:focus {
        outline: none;
      }

      $ button i {
        font-family: 'Material Icons';
        font-weight: normal;
        font-style: normal;
        font-size: 24px;
        line-height: 1;
        letter-spacing: normal;
        text-transform: none;
        display: inline-block;
        white-space: nowrap;
        word-wrap: normal;
        direction: ltr;
      }
    `;
  }
}

Component.register(MdIconButton);

//-----------------------------------------------------------------------------
// Text
//-----------------------------------------------------------------------------

export class MdText extends Component {
  visible() {
    return this.state;
  }

  render() {
    let text = this.state;
    if (text) {
      return `${Component.escape(text)}`;
    } else {
      return "";
    }
  }
}

Component.register(MdText);

export class MdLink extends Component {
  render() {
    if (!this.state) return "";
    let url = this.state.url;
    let text = this.state.text;
    let attrs = [];
    if (this.props.newtab || this.state.newtab) {
      attrs.push('target="_blank"');
    }
    if (this.props.external || this.state.external) {
      attrs.push('rel="noreferrer"');
    }
    if (this.props.notab || this.state.notab) {
      attrs.push('tabindex="-1"');
    }
    if (url == null) {
      return `<a>${Component.escape(text)}</a>`;
    } else if (text) {
      let extra = attrs.join(" ");
      return `<a href="${url}" ${extra}>${Component.escape(text)}</a>`;
    } else {
      return "";
    }
  }
}

Component.register(MdLink);

//-----------------------------------------------------------------------------
// Image
//-----------------------------------------------------------------------------

export class MdImage extends Component {
  visible() {
    return this.state;
  }

  render() {
    return `<img src="${this.state}" rel="noreferrer">`;
  }
}

Component.register(MdImage);

//-----------------------------------------------------------------------------
// Icon
//-----------------------------------------------------------------------------

export class MdIcon extends Component {
  constructor() {
    super();
    this.state = true;
  }

  visible() {
    return this.state;
  }

  render() {
    return `<i>${this.props.icon}</i>`;
  }

  static stylesheet() {
    return `
      $ i {
        font-family: 'Material Icons';
        font-weight: normal;
        font-style: normal;
        line-height: 1;
        letter-spacing: normal;
        text-transform: none;
        display: inline-block;
        white-space: nowrap;
        word-wrap: normal;
        direction: ltr;
      }
    `;
  }
}

Component.register(MdIcon);

//-----------------------------------------------------------------------------
// Radio button
//-----------------------------------------------------------------------------

export class MdRadioButton extends Component {
  render() {
    return `
      <input type="radio"
             name="${this.props.name}"
             value="${this.props.value}"
             ${this.props.selected ? "checked" : ""}>`;
  }

  static stylesheet() {
    return `
      $ {
        display: flex;
        height: 30px;
        width: 30px;
        border-radius: 50%;
      }

      $:hover {
        background-color: rgba(0,0,0,0.07);
      }

      $ input {
        height: 15px;
        width: 15px;
        margin: 8px;
        background: transparent;
        user-select: none;
        cursor: pointer;
      }
    `;
  }
}

Component.register(MdRadioButton);

//-----------------------------------------------------------------------------
// Input box
//-----------------------------------------------------------------------------

export class MdInput extends Component {
  value() {
    return this.find("input").value;
  }

  onupdated() {
    let value = this.state;
    if (value) this.find("input").value = value;
  }

  render() {
    let attrs = [];
    if (this.props.type) {
      attrs.push(` type="${this.props.type}"`);
    }
    if (this.props.placeholder) {
      attrs.push(` placeholder="${this.props.placeholder}"`);
    }
    attrs.push(' spellcheck="false"');
    if (this.props.autofocus != undefined) {
      attrs.push(' autofocus');
    }

    return `<input ${attrs.join("")}>`;
  }

  static stylesheet() {
    return `
      $ {
        display: block;
        position: relative;
        width: 100%;

        color: black;
        font-family: Roboto,Helvetica,sans-serif;
        font-size: 14px;
      }

      $ input {
        outline: none;
        border: none;
        line-height: 40px;
        height: 40px;
        width: 100%;
        padding: 10px;
        border-radius: 5px;
      }
    `;
  }
}

Component.register(MdInput);

//-----------------------------------------------------------------------------
// Search box
//-----------------------------------------------------------------------------

export class MdSearch extends Component {
  onconnected() {
    this.bind("input", "input", e => this.oninput(e));
    this.bind("input", "keydown", e => this.onkeydown(e));
    this.bind("input", "search", e => this.onsearch(e));
    this.bind(null, "focusin", e => this.onfocus(e));
    this.bind(null, "focusout", e => this.onunfocus(e));
    this.bind(null, "click", e => this.onclick(e));
  }

  onkeydown(e) {
    let list = this.find("md-search-list");
    if (list) {
      if (e.keyCode == 40) {
        list.next();
      } else if (e.keyCode == 38) {
        list.prev();
      } else if (e.keyCode == 13) {
        list.select(e.ctrlKey);
        e.preventDefault();
      }
    }
  }

  oninput(e) {
    let query = e.target.value;
    let min_length = this.props.min_length;
    if (min_length && query.length < min_length) {
      this.populate(null, null);
    } else {
      this.find("input").style.cursor = "wait";
      this.dispatchEvent(new CustomEvent("query", {detail: query}));
    }
  }

  onclick(e) {
    let item = e.target.closest("md-search-item");
    if (item) this.select(item, e.ctrlKey);
  }

  onsearch(e) {
    let list = this.find("md-search-list");
    if (list) list.select();
  }

  onfocus(e) {
    this.find("md-search-list").expand(true);
  }

  onunfocus(e) {
    this.find("md-search-list").expand(false);
  }

  populate(query, items) {
    // Ignore stale updates where the query does match the current value of the
    // search input box.
    if (query != null && query != this.query()) return;
    let list = this.find("md-search-list");
    list.update({items: items});
    list.scrollTop = 0;
    this.find("input").style.cursor = "";
  }

  select(item, keep) {
    if (!keep) this.find("md-search-list").expand(false);
    let input = this.find("input");
    if (item != null) {
      input.blur();
      this.dispatchEvent(new CustomEvent("item", {detail: item.props.value}));
    }
  }

  query() {
    return this.find("input").value;
  }

  clear() {
    let input = this.find("input");
    input.value = null;
    this.populate(null, null);
    input.focus();
  }

  render() {
    let attrs = [];
    if (this.props.placeholder) {
      attrs.push(` placeholder="${this.props.placeholder}"`);
    }
    attrs.push(' spellcheck="false"');
    if (this.props.autofocus != undefined) {
      attrs.push(' autofocus');
    }

    return `
        <input type="search" ${attrs.join("")}>
        <md-search-list></md-search-list>
    `;
  }

  static stylesheet() {
    return `
      $ {
        display: block;
        position: relative;
        width: 100%;

        color: black;
        font-family: Roboto,Helvetica,sans-serif;
        font-size: 15px;
      }

      $ input {
        outline: none;
        border: none;
        width: 100%;
        padding: 10px;
        border-radius: 5px;
        font-size: 15px;
      }
    `;
  }
}

Component.register(MdSearch);

export class MdSearchList extends Component {
  constructor() {
    super();
    this.bind(null, "mousedown", this.onmousedown);
    this.active = null;
  }

  onmousedown(e) {
    // Prevent search list from receiving focus on click.
    e.preventDefault();
  }

  expand(expanded) {
    if (!this.state || !this.state.items || this.state.items.length == 0) {
      expanded = false;
    }
    this.style.display = expanded ? "block" : "none";
  }

  next() {
    if (this.active) {
      if (this.active.nextSibling) {
        this.activate(this.active.nextSibling);
      }
    } else if (this.firstChild) {
      this.activate(this.firstChild);
    }
  }

  prev() {
    if (this.active) {
      this.activate(this.active.previousSibling);
    }
  }

  select(keep) {
    if (!this.active) this.next();
    this.match("md-search").select(this.active, keep);
  }

  activate(item) {
    if (this.active) {
      this.active.highlight(false);
    }

    if (item) {
      item.highlight(true);
      item.scrollIntoView({block: "nearest"});
    }
    this.active = item;
  }

  onupdated() {
    this.active = null;
  }

  render() {
    if (!this.state || !this.state.items || this.state.items.length == 0) {
      this.expand(false);
      return "";
    } else {
      this.expand(true);
      return this.state.items;
    }
  }

  static stylesheet() {
    return `
      $ {
        display: none;
        position: absolute;
        background: #ffffff;
        box-shadow: rgba(0, 0, 0, 0.16) 0px 2px 4px 0px,
                    rgba(0, 0, 0, 0.23) 0px 2px 4px 0px;
        z-index: 99;
        width: 100%;
        max-height: 400px;
        overflow: auto;
        cursor: pointer;
      }
    `;
  }
}

Component.register(MdSearchList);

export class MdSearchItem extends Component {
  onconnected() {
    this.bind(null, "mousemove", this.onmousemove);
  }

  onmousemove(e) {
    this.match("md-search-list").activate(this);
  }

  highlight(on) {
    this.style.background = on ? "#f0f0f0" : "#ffffff";
  }

  static stylesheet() {
    return `
      $ {
        display: block;
        border-top: 1px solid #d4d4d4;
        paddding-bottom: 1px;
      }
    `;
  }
}

Component.register(MdSearchItem);

//-----------------------------------------------------------------------------
// Data table
//-----------------------------------------------------------------------------

export class MdDataField extends Component {}

Component.register(MdDataField);

export class MdDataTable extends Component {
  constructor() {
    super();
    this.fields = [];
    for (const e of this.elements) {
      this.fields.push({
        name: e.getAttribute("field"),
        header: e.innerHTML,
        style: e.style ? e.style.cssText : null,
        escape: !e.getAttribute("html"),
      });
    }
  }

  render() {
    let h = [];
    h.push("<table><thead><tr>");
    for (const fld of this.fields) {
      if (fld.style) {
        h.push(`<th style="${fld.style}">`);
      } else {
        h.push("<th>");
      }
      h.push(fld.header);
      h.push("</th>");
    }
    h.push("</tr></thead><tbody>");

    if (this.state) {
      for (const row of this.state) {
        if (row.style) {
          h.push(`<tr style="${row.style}">`);
        } else {
          h.push("<tr>");
        }
        for (const fld of this.fields) {
          if (fld.style) {
            h.push(`<td style="${fld.style}">`);
          } else {
            h.push("<td>");
          }

          let value = row[fld.name];
          if (value == undefined) value = "";
          value = value.toString();

          if (fld.escape) value = Component.escape(value);
          h.push(value);
          h.push("</td>");
        }
        h.push("</tr>");
      }
    }

    h.push("</tbody></table>");

    return h.join("");
  }

  static stylesheet() {
    return `
      $ {
        border: 0;
        white-space: nowrap;
        font-size: 14px;
        text-align: left;
      }

      $ thead {
        padding-bottom: 3px;
      }

      $ th {
        vertical-align: bottom;
        padding: 8px 12px;
        box-sizing: border-box;
        border-bottom: 1px solid rgba(0,0,0,.12);
        text-overflow: ellipsis;
        color: rgba(0,0,0,.54);
      }

      $ td {
        vertical-align: middle;
        border-bottom: 1px solid rgba(0,0,0,.12);
        padding: 8px 12px;
        box-sizing: border-box;
        text-overflow: ellipsis;
        overflow: hidden;
      }

      $ td:first-of-type, $ th:first-of-type {
        padding-left: 24px;
      }
    `;
  }
}

Component.register(MdDataTable);

