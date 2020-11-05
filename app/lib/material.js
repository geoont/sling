// Material Design web components.

import {Component, stylesheet} from "/common/lib/component.js";

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
  line-height: 20px;
  padding: 0;
  margin: 0;
  box-sizing: border-box;

  width: 100%;
  height: 100%;
  min-height: 100%;
  position:relative;
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
        background-color: rgb(250,250,250);

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
        align-items: center;
        background-color: #FFFFFF;
        color: #000000;
        height: 56px;
        max-height: 56px;
        font-size: 24px;
        margin: 0;
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
      <svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 283.46 176.46">
        <g><path d="${logo}"/></g>
      </svg>`;
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
  render() {
    return this.html`
      <button ${this.props.disabled ? "disabled" : ""}>
        <i>${this.props.icon}</i>
      </button>`;
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
        margin: 0 6px;
        padding: 8px;
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

      md-toolbar $ button:first-child {
        margin-left: -8px;
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
  render() {
    let text = this.state;
    if (text) {
      return `<div>${Component.escape(text)}</div>`;
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
    let extra = "";
    if (this.props.newtab || this.state.newtab) extra = 'target="_blank"';
    if (text) {
      return `<a href="${url}" ${extra}>${Component.escape(text)}</a>`;
    } else {
      return "";
    }
  }
}

Component.register(MdLink);

//-----------------------------------------------------------------------------
// Icon
//-----------------------------------------------------------------------------

export class MdIcon extends Component {
  render() {
    return `<i>${this.props.icon}</i>`;
  }

  static stylesheet() {
    return `
      $ {
        align-items: center;
        display: flex;
      }

      $ i {
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
// Search box
//-----------------------------------------------------------------------------

export class MdSearch extends Component {
  onconnected() {
    this.bind("input", "input", e => this.oninput(e));
    this.bind("input", "keydown", e => this.onkeydown(e));
    this.bind(null, "focus", e => this.onfocus(e));
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
        list.select();
        e.preventDefault();
      }
    }
  }

  oninput(e) {
    let query = e.target.value;
    let min_length = this.props.min_length;
    if (min_length && query.length < min_length) {
      this.populate(null);
    } else {
      this.dispatchEvent(new CustomEvent("query", {detail: query}));
    }
  }

  onclick(e) {
    let item = e.target.closest("md-search-item");
    if (item) this.select(item);
  }

  onfocus(e) {
    this.find("md-search-list").expand(true);
  }

  onunfocus(e) {
    this.find("md-search-list").expand(false);
  }

  populate(items) {
    let list = this.find("md-search-list");
    list.update({items: items});
    list.scrollTop = 0;
  }

  select(item) {
    this.find("md-search-list").expand(false);
    if (item != null) {
      if (item.props.name) {
        this.find("input").value = item.props.name;
      }
      this.dispatchEvent(new CustomEvent("item", {detail: item.props.value}));
    }
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

Component.register(MdSearch);

export class MdSearchList extends Component {
  constructor() {
    super();
    this.bind(null, "mousedown", this.onmousedown);
    this.active = -1;
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
    if (this.active < this.childElementCount - 1) {
      if (this.active != -1) this.deactivate(this.active);
      this.active++;
      this.activate(this.active);
    }
  }

  prev() {
    if (this.active >= 0) {
      if (this.active != -1) this.deactivate(this.active);
      this.active--;
      if (this.active >= 0) {
        this.activate(this.active);
      }
    }
  }

  select() {
    let item = this.active == -1 ? null : this.children[this.active];
    this.match("md-search").select(item);
  }

  activate(n) {
    this.children[n].classList.add("active");
    this.children[n].scrollIntoView({block: "nearest"});
  }

  deactivate(n) {
    this.children[n].classList.remove("active");
  }

  onupdated() {
    this.active = -1;
  }

  render() {
    if (!this.state || !this.state.items || this.state.items.length == 0) {
      this.expand(false);
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
        background-color: white;
        box-shadow: rgba(0, 0, 0, 0.16) 0px 2px 4px 0px,
                    rgba(0, 0, 0, 0.23) 0px 2px 4px 0px;
        z-index: 99;
        width: 100%;
        max-height: 400px;
        overflow: auto;
        cursor: pointer;
      }

      $ .active {
        background-color: #d0d0d0;
      }
    `;
  }
}

Component.register(MdSearchList);

export class MdSearchItem extends Component {
  static stylesheet() {
    return `
      $ {
        display: block;
        border-top: 1px solid #d4d4d4;
      }

      $:hover {
        background-color: #f0f0f0;
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
      if (e instanceof MdDataField) {
        this.fields.push({
          name: e.props.field,
          header: e.innerHTML,
          style: e.style ? e.style.cssText : null,
          escape: !e.props.html,
        });
      }
    }
  }

  render() {
    let out = [];
    out.push("<table><thead><tr>");
    for (const fld of this.fields) {
      if (fld.style) {
        out.push(`<th style="${fld.style}">`);
      } else {
        out.push("<th>");
      }
      out.push(fld.header);
      out.push("</th>");
    }
    out.push("</tr></thead><tbody>");

    if (this.state) {
      for (const row of this.state) {
        out.push("<tr>");
        for (const fld of this.fields) {
          if (fld.style) {
            out.push(`<td style="${fld.style}">`);
          } else {
            out.push("<td>");
          }

          let value = row[fld.name];
          if (value == undefined) value = "";
          value = value.toString();

          if (fld.escape) value = Component.escape(value);
          out.push(value);
          out.push("</td>");
        }
        out.push("</tr>");
      }
    }

    out.push("</tbody></table>");

    return out.join("");
  }

  static stylesheet() {
    return `
      $ {
        border: 0;
        border-collapse: collapse;
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
      }

      $ td:first-of-type, $ th:first-of-type {
        padding-left: 24px;
      }
    `;
  }
}

Component.register(MdDataTable);
