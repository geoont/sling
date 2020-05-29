#!/bin/bash

LANGUAGE=${LANGUAGE:-en}
PORT=${PORT:-8080}

SPEC='{
  annotator: "clear"
  annotator: "parser"
  ;annotator: "prune-nominals"
  annotator: "mention-name"

  inputs: {
    parser: {
      ;file: "local/data/e/caspar/caspar.flow"
      ;file: "local/data/e/knolex/knolex-en.flow"
      ;file: "local/data/e/knolex/bio-en.flow"
      file: "local/data/e/knolex/biaf-en.flow"
      format: "flow"
    }
    commons: {
      ;file: "local/data/e/wiki/kb.sling"
      file: "data/dev/types.sling"
      format: "store/frame"
    }
  }
  parameters: {
    language: "LANG"
  }
}'

bazel-bin/sling/nlp/document/corpus-browser \
  --commons data/dev/types.sling \
  --spec "${SPEC//LANG/$LANGUAGE}" \
  --port $PORT $@ \
  local/data/e/wiki/$LANGUAGE/documents@10.rec

